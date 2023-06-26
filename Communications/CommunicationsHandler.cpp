#include "CommunicationsHandler.h"
#include <stdlib.h>
#include "stdio.h"
#include "Generics.h"

#if USE_ACTORS == 1
	#if USE_VECTORS == 1
    	std::vector<Actor *> CommandHandler::Actors;
	#else
		ActorWrapper * CommandHandler::Actors = NULL;
		uint8_t CommandHandler::Count = 0;
	#endif
#endif

unsigned long * CommandHandler::UID = NULL;
#if USE_WATCH_DOG ==1
	IWDG_HandleTypeDef * CommandHandler::hnl_watchdog = NULL;
#endif

//Buffers
uint8_t 			*CommandHandler::CommsBuffer = NULL;
uint8_t 			*CommandHandler::buffer = NULL;
volatile Command 	*CommandHandler::command = new Command();
unsigned char 		 CommandHandler::currentByte = 0;
bool 	 			 CommandHandler::FirstRun = true;

//Event Ticks
uint32_t 			 CommandHandler::tickstart;
uint32_t 			 CommandHandler::tickHdnl;
uint32_t 			 CommandHandler::LastReceived;

//Handles
UART_HandleTypeDef 	*CommandHandler::handle = NULL;
GPIO_TypeDef		*CommandHandler::TXEnablePort = NULL;
int16_t				 CommandHandler::TXEnablePin = 0;

//System State
uint8_t GData = 0;
bool 	CommandHandler::Initalised = false;
bool 	CommandHandler::UnprocessedCommand = false;
bool 	CommandHandler::LineBusy = false;
bool 	CommandHandler::HasTransmitEnable = false;
CommsReceiveState CommandHandler::ReceiveState = RECV_READY;

//Memory Controller
#if USE_MEMORY_CONTROLLER == 1
	W25X40CL 		*CommandHandler::Memory =NULL;
	uint32_t 		 CommandHandler::StartAddress = 0;
	uint8_t 		 CommandHandler::BufferObjects = 0;
#endif

Command::Command(){
	this->Data = &GData;
	this->Length = 0;
}
void CommandHandler::Initalise(UART_HandleTypeDef *Handle, uint8_t * CommBuffer){
	handle = Handle;
	CommsBuffer=CommBuffer;
	Initalised = true;
	LineBusy = false;
	LastReceived = 0;
}
void CommandHandler::Initalise(UART_HandleTypeDef *Handle, uint8_t * CommBuffer, unsigned long * uid){
	Initalise(Handle, CommBuffer);
	UID = uid;
	LineBusy = false;
	LastReceived = 0;
}
void CommandHandler::Initalise(UART_HandleTypeDef *Handle, uint8_t *CommBuffer, unsigned long *uid, GPIO_TypeDef *TXEnablePort, int16_t TXEnablePin){
	Initalise(Handle, CommBuffer);
	HasTransmitEnable = true;
	CommandHandler::TXEnablePort = TXEnablePort;
	CommandHandler::TXEnablePin = TXEnablePin;
	UID = uid;
	LineBusy = false;
	LastReceived = 0;
}
#if USE_MEMORY_CONTROLLER == 1
	void CommandHandler::SetupMemoryController(W25X40CL * MemoryCtrl, uint32_t StartMemoryAddress){
		Memory = MemoryCtrl;
		StartAddress = StartMemoryAddress;
	}

#endif
#if USE_ACTORS == 1
    void CommandHandler::AddActor(Actor * NewActor){
		#if USE_VECTORS == 1
        	Actors.push_back(NewActor);
		#else
			if (Count == 0){ Actors = (ActorWrapper *)malloc(sizeof(ActorWrapper));}
			else{
				size_t newSize = Count + 1;
				Actors = (ActorWrapper *)realloc(Actors, newSize * sizeof(ActorWrapper));
			}
			(Actors + Count)->Link = NewActor;
			Count++;
		#endif
    }
    CommsState CommandHandler::PublishStates(ActorDirection States){
    	if(Initalised==false){ return COMMS_ERROR;}
		#if USE_VECTORS == 1
    		for(std::vector<Actor *>::size_type i = 0; i != Actors.size(); i++) {
    			Actor * Member = Actors[i];
        	    ActorDirection Dir = Member->GetDirection();
    			if ((Member->GetState() == true)&&(((States & ActorDirection::Output) == Dir) || ((States & ActorDirection::Input) == Dir))){
    				String * Buffer = new String();
					#if USE_ADDRESSING == 1
						if (Member->BuildCommandString(Buffer, UID) == ACTOR_OK){
					#else
						if (Member->BuildCommandString(Buffer) == ACTOR_OK){
					#endif
    						TransmitString(*Buffer);
    					}
    					Member->ResetState();
    			}
    		}
		#else
			for(uint8_t i=0; i<Count; i++){
        		Actor * Member = (Actors + i)->Link;
        		ActorDirection Dir = Member->GetDirection();
    			if ((Member->GetState() == true)&&(((States & ActorDirection::Output) == Dir) || ((States & ActorDirection::Input) == Dir))){
    				String Buffer = "";
    				#if USE_ADDRESSING == 1
						if (Member->BuildCommandString(&Buffer, UID) == ACTOR_OK){
					#else
						if (Member->BuildCommandString(&Buffer) == ACTOR_OK){
					#endif
							TransmitString(Buffer);
    					}
    				Member->ResetState();
    			}
    		}
		#endif
    	return COMMS_OK;
    }
	CommsState CommandHandler::PublishActor(Actor * Input){
		if(Initalised==false){ return COMMS_ERROR;}
		if(Input == NULL){ return COMMS_ERROR;}
		#if USE_VECTORS == 0
			String Buffer = "";
			if (Input->BuildCommandString(&Buffer) == ACTOR_OK){
				TransmitString(Buffer);
			}
		#endif
		return COMMS_OK;
	}
#endif
void CommandHandler::EnableTransmit(bool State){
	if (HasTransmitEnable == false){ return; }
	switch(State){
		case true:
			HAL_GPIO_WritePin(TXEnablePort, TXEnablePin, GPIO_PIN_SET); break;
		case false:
			HAL_GPIO_WritePin(TXEnablePort, TXEnablePin, GPIO_PIN_RESET); break;
	}
}

/* 
	Typical Command String:
	C:<NAME>=<ASSIGNMENT>;
	With Address
	C:<THIS ID>,<NAME>=<ASSIGNMENT>;
	With Checksum
	C:<THIS ID>,<NAME>=<ASSIGNMENT><SINGLE CHAR SUM>;
	C:<THIS ID>,<NAME>=<ASSIGNMENT><SINGLE CHAR SUM>;
	Example:
	C:TEMP=12.355;  (No address or checksum, rendered as analog value)
	Time Example:
	C:TIME=YYYMMDDHHMMSS;
	C:TIME=0220410203000;
	Year support for 2000->2255
*/
CommsState CommandHandler::ProcessCommand(uint8_t * Input){
	uint8_t Symbol = *Input;
	CommsState ret = COMMS_OK;
	LastReceived = HAL_GetTick();
	LineBusy = false;
	switch(ReceiveState){
		case RECV_READY:
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Ready\n");
			#endif 
			if (Symbol == COMMAND_MARK_IN){ ReceiveState = RECV_ARMMED; tickstart = HAL_GetTick();}
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Current Byte = %s\n",Input);
			#endif
			return ret;
		case RECV_ARMMED:
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Armed\n");
			#endif 
			if(Symbol == COMMAND_MARK_INIT){ ReceiveState = RECV_INCOMMAND; currentByte = 0;}
			else{ ReceiveState = RECV_READY;}
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Current Byte = %s\n",Input);
			#endif
			return ret;
		case RECV_INCOMMAND:
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Appending Symbol %d\n",currentByte);
			#endif 
			if (currentByte < MAX_COMMAND_LENGTH){
				if (Symbol == COMMAND_MARK_OUT){ ReceiveState = RECV_COMPLETE; }
				else{
					if (currentByte == 0){ 
						buffer = (uint8_t *)realloc(buffer,sizeof(uint8_t));//(uint8_t*)malloc(sizeof(uint8_t));
					}
				    uint8_t * temp = (uint8_t *)realloc(buffer, (currentByte + 1) *sizeof(uint8_t));
				    if (temp == NULL){ ReceiveState = RECV_READY; currentByte = 0; ret = COMMS_ERROR; break;}
				    buffer = temp;
				    *(buffer + currentByte) = Symbol;
					currentByte++;
				}
			}
			else{ ReceiveState = RECV_READY; currentByte = 0; 
				TransmitErrorStatus(true,ERROR_COMMAND_TOO_LONG);
				ret = COMMS_ERROR;
			}
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Current Byte = %s\n",Input);
			#endif
			break;
		default:
			break;
	}
	if (ReceiveState == RECV_COMPLETE){
		#if COMMSHDLR_DEBUG_MESSAGES == 1
			printf("Command: Processed\n");
		#endif
		//free(command->Data);
		if (FirstRun== true){
			FirstRun=false;
			command->Data = (uint8_t*)malloc(sizeof(uint8_t)*(currentByte+1)); //currentByte +1
		}
		else{
			command->Data = (uint8_t*)realloc(command->Data, sizeof(uint8_t)*(currentByte+1));//currentByte 
		}
		memcpy(command->Data,buffer,sizeof(uint8_t)*(currentByte+1));
		command->Length = currentByte+1;
		buffer = (uint8_t *)realloc(buffer,sizeof(uint8_t));
		UnprocessedCommand = true;
		ReceiveState = RECV_READY;
	}
	return ret;
}
void CommandHandler::CheckTimeOuts(){
	uint32_t tickend = HAL_GetTick();
	if (LineBusy == false){
		#if USE_MEMORY_CONTROLLER == 1
			if (Memory != NULL){
				if (BufferObjects > 0){
					for(int i = 0;i<BufferObjects;i++){
						uint32_t Address = StartAddress + (COMMS_MEMORY_STEP * i);
						String TxBuffer = "";
						Memory->Read(Address,&TxBuffer);
						if (TxBuffer != ""){
							TransmitString(TxBuffer);
						}
					}
					BufferObjects = 0;
				}
			}
		#endif
	}
	LineBusy = false;
	if (ReceiveState != RECV_READY){
		if (tickend - tickstart > MAX_TIMEOUT){
			ReceiveState = RECV_READY;
			currentByte = 0;
			buffer = (uint8_t *)realloc(buffer,sizeof(uint8_t));
			HAL_UART_Receive_IT(handle, CommsBuffer, 1);
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Timed Out\n");
			#endif 
			TransmitErrorStatus(true,ERROR_COMMAND_TIMEOUT);
		}
		else{
			if (tickend < tickstart){tickstart = tickend;}
		}
	}
	if(tickend - tickHdnl > 1000){
		tickHdnl = tickend;
		HAL_UART_Receive_IT(handle, CommsBuffer, 1);
		#if COMMSHDLR_DEBUG_MESSAGES == 1
			printf("Command: Handle refreshed.\n");
		#endif 
	}
	else {
		if (tickend < tickHdnl){tickHdnl = tickend;}
	}
	LineBusy = false;
}
#if USE_ACTORS == 1
	uint8_t CommandHandler::FindActor(String & Name){
		uint8_t ret = 255;
		#if USE_VECTORS == 1
			for(std::vector<Actor *>::size_type i = 0; i != Actors.size(); i++) {
    			Actor * Member = Actors[i];
        	    if (*Member->GetName() == Name){ ret = i; break; }
    		}
		#else
			for(uint8_t i = 0; i<Count; i++){
				Actor * Member = (Actors+i)->Link;
				if ((*(Member->GetName()) == Name) && (Member->IsEnabled() == true)){
					ret = i;
					break;
				}
			}
		#endif
		return ret;
	}
	void CommandHandler::EnableCategory(uint8_t Category, bool State){
		#if USE_VECTORS == 1

		#else
			if(Category == 0){return;}
			for(uint8_t i = 0; i<Count; i++){
				Actor * Member = (Actors+i)->Link;
				if (Member != NULL){
					if ((Member->Category == Category)){
						Member->Enable(State);
					}
				}
			}
		#endif
	}
	void CommandHandler::ParseCommand(){
		if (UnprocessedCommand ==  true){
			bool IncorrectCheckSum = false;
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Parsing...\n");
			#endif
			String UnprocessedString = Defines::EmptyString;
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Bytes to process: %d\n",command->Length);
			#endif
			#if USE_CHECKSUM == 1
				uint8_t CheckSum = 0;
			#endif
			uint8_t CommandLength = command->Length - 1;
			UnprocessedCommand = false;
			//Append and Calculate Check Sum
			for(uint8_t i = 0;i<CommandLength;i++){
				char Byte = *(command->Data + i);
				#if COMMSHDLR_DEBUG_MESSAGES == 1
					printf("Command: Processing: %c\n",Byte);
				#endif
				#if USE_CHECKSUM == 1
					if(i == (CommandLength - 1)){
						if (CheckSum == 0x3B) { CheckSum+=CHECKSUM_TERM_OFFSET; }
						if (CheckSum == 0x00) { CheckSum+=CHECKSUM_TERM_MINOROFFSET; }
						if (CheckSum != *(command->Data + i)){
							#if COMMSHDLR_DEBUG_MESSAGES == 1
								printf("Command: Checksum mismatch: %c\n",Byte);
							#endif
							//TransmitErrorStatus(true,ERROR_COMMAND_INCORRECT_CHKSUM);
							//return;
							IncorrectCheckSum = true;
						}
					}
					else{
						UnprocessedString += Byte;
						CheckSum += Byte;
					}
				#else
					UnprocessedString += Byte;
				#endif
			}
			//Release Buffer
			command->Data = (uint8_t *)realloc(command->Data,sizeof(uint8_t));
			command->Length = 0;
			//Parse Command
			if(UnprocessedString.startsWith("ID,")){
				String Assignment(UnprocessedString.split(Defines::Comma,1));
				if(UID == NULL){ return;}
				if (IncorrectCheckSum == true){
					TransmitErrorStatus(true,ERROR_COMMAND_INCORRECT_CHKSUM);
					return;
				}
				if (Duplex == DUPLEX_HALF){LineBusy = false;}
				uint16_t Delay = ((*UID) & 0xFF) * 100;
				HAL_Delay(Delay);
				String Buffer = "ID," + Assignment + Defines::Equals + String(*UID,10);
				TransmitString(Buffer);
			}
			else{
				#if USE_ADDRESSING == 1
					if (CheckAddress(UnprocessedString) == false){return;}
					if (IncorrectCheckSum == true){
						TransmitErrorStatus(true,ERROR_COMMAND_INCORRECT_CHKSUM);
						return;
					}
					String Payload(UnprocessedString.split(Defines::Comma,1));
					if (AssignActor(Payload) == true){
						TransmitErrorStatus(false,STATUS_COMMAND_SUCCESSFUL);
						return;
					}
					else{ TransmitErrorStatus(true,ERROR_COMMAND_ACTOR_NOT_EXISTS); return;}
				#else
					if (IncorrectCheckSum == true){
						TransmitErrorStatus(true,ERROR_COMMAND_INCORRECT_CHKSUM);
						return;
					}
					if (AssignActor(UnprocessedString) == true){ TransmitErrorStatus(false,STATUS_COMMAND_SUCCESSFUL); return;}
					else{ TransmitErrorStatus(true,ERROR_COMMAND_ACTOR_NOT_EXISTS); return;}
				#endif
			}
		}
	}
	bool CommandHandler::AssignActor(String & Payload){
		if (Payload.contains(Defines::Equals)==true){
			String Name(Payload.split(Defines::Equals,0));
			String Value(Payload.split(Defines::Equals,1));
			uint8_t Index = FindActor(Name);
			if (Index == 255){
				#if COMMSHDLR_DEBUG_MESSAGES == 1
					printf("Command: Actor not found!\n");
				#endif
				return false;
			}
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Actor Found @ %d\n",Index);
			#endif
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Actor Data Setting...\n");
			#endif
			#if USE_VECTORS == 1
				Actors[Index]->SetData(Value,true);
			#else
				(Actors+Index)->Link->SetData(Value,true);
			#endif
		}
		else{
			uint8_t Index = FindActor(Payload);
			if (Index == 255){
				#if COMMSHDLR_DEBUG_MESSAGES == 1
					printf("Command: Actor not found!\n");
				#endif
				return false;
			}
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Actor Found @ %d\n",Index);
			#endif
			#if COMMSHDLR_DEBUG_MESSAGES == 1
				printf("Command: Actor Invoking\n");
			#endif
			#if USE_VECTORS == 1
				Actors[Index]->SetData(true,true);
			#else
				(Actors+Index)->Link->SetData(true,true);
			#endif
		}
		return true;
	}
	void CommandHandler::InvalidateAll(){
		#if USE_VECTORS == 1
    		for(std::vector<Actor *>::size_type i = 0; i != Actors.size(); i++) {
    			Actor * Member = Actors[i];
        	    Member->Invalidate();
    		}
		#else
			for(uint8_t i=0; i<Count; i++){
        		Actor * Member = (Actors + i)->Link;
        		Member->Invalidate();
    		}
		#endif
	}
#endif
void CommandHandler::TransmitString(char * Message){
	String Output = String(Message);
	TransmitString(Output);
}
void CommandHandler::TransmitString(String & Message){
	if (Duplex == DUPLEX_HALF){
		if (LineBusy == true){
			//Push Data to Buffer
			#if USE_MEMORY_CONTROLLER == 1
				#if COMMSHDLR_DEBUG_MESSAGES == 1
					printf("Saving to Buffer @%d\n", BufferObjects);
				#endif 
				if (Memory != NULL){
					uint32_t Address = StartAddress + (COMMS_MEMORY_STEP * BufferObjects);
					Memory->EraseSector(Address);
					Memory->Write(Address, Message);
					BufferObjects++;
				}
			#endif
			return;
		}
	}
	Transmit(Message);
}
void CommandHandler::Transmit(String & Message){
	EnableTransmit(true);
	#if USE_CHECKSUM == 1
		uint8_t CheckSum =0;
		if (Message.length() >= MAX_TRANSMITLENGTH){return;}
		for(uint16_t i = 0;i<Message.length();i++){
			CheckSum+=Message[i];
		}
		if (CheckSum == 0x3B) { CheckSum+=CHECKSUM_TERM_OFFSET; }
		if (CheckSum == 0x00) { CheckSum+=CHECKSUM_TERM_MINOROFFSET; }
		String Buffer = Defines::CommandHeader + Message + (char)CheckSum + Defines::Term;
	#else
		String Buffer = Defines::CommandHeader + Message + Defines::Term;
	#endif
	unsigned int Length = Buffer.length() + 1;
    char TempString[Length];
	Buffer.toCharArray(TempString, Length);
	#if USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    HAL_UART_Transmit(handle, (uint8_t *)&TempString, Length, MAX_TRANSMITTIMEOUT);//HAL_MAX_DELAY
	#if USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
	EnableTransmit(false);
}
void CommandHandler::TransmitErrorStatus(bool IsError, uint8_t ErrorCode){
	#if USE_ADDRESSING == 1
		if (IsError == true){
			String Buffer = String(*UID,10) + Defines::Comma + Defines::ErrorHeader + Defines::Equals + String(ErrorCode,10);
			TransmitString(Buffer);
		}
		else{
			String Buffer = String(*UID,10) + Defines::Comma + Defines::StatusHeader + Defines::Equals + String(ErrorCode,10);
			TransmitString(Buffer);
		}
		
	#else
		if (IsError == true){
			String Buffer = Defines::ErrorHeader + Defines::Equals + String(ErrorCode,10);
			TransmitString(Buffer);
		}
		else{
			String Buffer = Defines::StatusHeader + Defines::Equals + String(ErrorCode,10);
			TransmitString(Buffer);
		}
	#endif
}
bool CommandHandler::CheckAddress(String & Payload){
	if(UID == NULL){ return false;}
	if(Payload.contains(Defines::Comma)==false){ 
		#if COMMSHDLR_DEBUG_MESSAGES == 1
			printf("Command: Mismatch command format!\n");
		#endif
		return false;
	}
	String ID(Payload.split(Defines::Comma,0));
	if((unsigned long)ID.toInt() == *UID){
		#if COMMSHDLR_DEBUG_MESSAGES == 1
			printf("Command: ID did not match!\n");
		#endif
		return true; 
	}
	return false;
}
CommsDuplex CommandHandler::Duplex = DUPLEX_FULL;
void CommandHandler::SetDuplex(CommsDuplex duplex){
	Duplex = duplex;
}


void CommandHandler::AppendNumber(String & Input, int Value, uint8_t Padding){
	String ValueString = String(Value);
	int AppendLength = Padding - ValueString.length();
	if (AppendLength >= 1){
		for(int i=0;i<AppendLength;i++){
			Input += '0';
		}
	}
	Input += ValueString;
}

#if USE_WATCH_DOG ==1
	void CommandHandler::RefreshWatchDog(){
		if (hnl_watchdog == NULL){ return; }
		HAL_IWDG_Refresh(hnl_watchdog);
	}
	void CommandHandler::LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog){
		hnl_watchdog = HandleWatchDog;
		RefreshWatchDog();
	}
#endif