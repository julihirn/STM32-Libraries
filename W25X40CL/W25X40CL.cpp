#include "W25X40CL.h"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#if W25X40CL_USE_SPIHANDLER == 1
	W25X40CL::W25X40CL(Handler Bus, GPIO_TypeDef *Port, uint16_t Pin) {
		this->Port = Port;
		this->Pin = Pin;
		this->Bus = Bus;
        Deselect();
        DeviceFlags |= W25X40CL_FLAG_ISCONFIG;
	}
    W25X40CL::W25X40CL(Handler Bus, GPIO_TypeDef *Port, uint16_t Pin, GPIO_TypeDef *WriteProtectPort, uint16_t WriteProtectPin) {
		this->Port = Port;
		this->Pin = Pin;
		this->Bus = Bus;
        Deselect();
        this->WPPort = WriteProtectPort;
        this->WPPin = WriteProtectPin;
        WriteProtect(false);
        DeviceFlags |= W25X40CL_FLAG_ISCONFIG;
	}
#else
	W25X40CL::W25X40CL(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin){
		this->Port = Port;
		this->Pin = Pin;
		this->SPIHandle = Handle;
		Deselect();
        DeviceFlags |= W25X40CL_FLAG_ISCONFIG;
	}
    W25X40CL::W25X40CL(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin, GPIO_TypeDef *WriteProtectPort, uint16_t WriteProtectPin){
		this->Port = Port;
		this->Pin = Pin;
		this->SPIHandle = Handle;
		Deselect();
        this->WPPort = WriteProtectPort;
        this->WPPin = WriteProtectPin;
        WriteProtect(false);
        DeviceFlags |= W25X40CL_FLAG_ISCONFIG;
	}
#endif
void W25X40CL::Deselect(){
	if (Port != NULL){
		HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
	}
}
void W25X40CL::SendCommand(W25X40CLCommand Command){
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    this->DeviceFlags |= W25X40CL_FLAG_ISBUSY;
    uint8_t tx_buffer = (uint8_t)Command;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Invoking Command...\n");
        printf("   Command Byte:\n");
        printf("      0x%02X\n", (unsigned char)tx_buffer);
	#endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,tx_buffer,10000);
        SPIHandler::EndTransaction(this->Bus);
    #else
		HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle,&tx_buffer,sizeof(uint8_t),1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
      
    #endif  
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    DeviceFlags &= ~W25X40CL_FLAG_ISBUSY;
}

void W25X40CL::UseTransactions(bool State){
    AllowTransactions = State;
}
void W25X40CL::StartWriteTransaction(uint32_t Address){
    if (AllowTransactions == false){return;}
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    SendCommand(W25X40CLCommand::WriteEnable);
    this->DeviceFlags |= W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY | W25X40CL_FLAG_ISTRANSACTION;
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::PageProgram,1000);
        SendAddress(Address);
    #else
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        uint8_t TxBuffer[4] = {};
        TxBuffer[0] = (uint8_t)W25X40CLCommand::PageProgram;
        TxBuffer[1] = (Address >> 16) & 0xFF;
        TxBuffer[2] = (Address >> 8) & 0xFF;
        TxBuffer[3] = Address & 0xFF;
        uint16_t CommunicationLength = sizeof(uint8_t) * 4;
        HAL_SPI_Transmit(SPIHandle, TxBuffer, CommunicationLength,1);
    #endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
}
void W25X40CL::EndTransaction(){
    if (AllowTransactions == false){return;}
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::EndTransaction(this->Bus);
    #else
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    #endif
    this->DeviceFlags &= ~(W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY);
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Awaiting...\n");
	#endif
    while(IsReady() == false){HAL_Delay(1); }
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("IsReady!\n");
	#endif
    this->DeviceFlags &= ~(W25X40CL_FLAG_ISTRANSACTION);
}

uint32_t W25X40CL::WriteTransaction(bool Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: bool, With data: %d\n", (char)Data);
    #endif
    return WriteTransaction((uint8_t*)&Data, 1);
}
uint32_t W25X40CL::WriteTransaction(char Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: char, With data: %d\n", Data);
    #endif
    return WriteTransaction((uint8_t*)&Data, 1);
}
uint32_t W25X40CL::WriteTransaction(int Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: int, With data: %d\n", Data);
    #endif
    union {
        int True;
        uint8_t Bytes[2];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 2);
}
uint32_t W25X40CL::WriteTransaction(long Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: float, With data: %ld\n", Data);
    #endif
    union {
        long True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 4);
}
uint32_t W25X40CL::WriteTransaction(float Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: float, With data: %f\n", Data);
    #endif
    union {
        float True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 4);
}
uint32_t W25X40CL::WriteTransaction(double Data){
    union {
        double True;
        uint8_t Bytes[8];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 8);
}

uint32_t W25X40CL::WriteTransaction(uint8_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: char, With data: %d\n", (int)Data);
    #endif
    return WriteTransaction(&Data, 1);
}
uint32_t W25X40CL::WriteTransaction(uint16_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: int, With data: %d\n", (int)Data);
    #endif
    union {
        uint16_t True;
        uint8_t Bytes[2];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 2);
}
uint32_t W25X40CL::WriteTransaction(uint32_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: long, With data: %ld\n", (long)Data);
    #endif
    union {
        uint32_t True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    return WriteTransaction(Value.Bytes, 4);
}


uint32_t W25X40CL::WriteTransaction(uint8_t *Data, uint16_t Length){
    if (AllowTransactions == false){return 0;}
    if((this->DeviceFlags & W25X40CL_FLAG_ISTRANSACTION)!=W25X40CL_FLAG_ISTRANSACTION){return 0;}
    #if W25X40CL_USE_SPIHANDLER == 1
        for(int i =0;i<Length;i++){
        	uint8_t val = *(Data+i);
            SPIHandler::WriteData(this->Bus, val,1000);
            #if W25X40CL_DEBUG_MESSAGES == 1
                printf("0x%04X - %02d\n", Address+i, (int)val);
            #endif
        }
    #else
        uint8_t *TxBuffer;
        TxBuffer = (uint8_t*)calloc(Length, sizeof(uint8_t));
        for(int i=0;i<Length;i++){
            uint8_t Var = *(Data+i);
        	*(TxBuffer+i) = Var;
            //*(Data+i) = Var;
			#if W25X40CL_DEBUG_MESSAGES == 1
            	printf("      %02d -   0x%02X\n", i, (unsigned char)Var);
			#endif
        }
        uint16_t CommunicationLength = sizeof(uint8_t) * Length;
        HAL_SPI_Transmit(SPIHandle, TxBuffer, CommunicationLength,1);
    #endif
    return Length;
}

void W25X40CL::WriteData(uint32_t Address, uint8_t *Data, uint16_t Length){
    if(Length == 0){return;}
    //if(Data != NULL){return;}
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    SendCommand(W25X40CLCommand::WriteEnable);
    this->DeviceFlags |= W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY;
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Writing Data...\n");
    #endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::PageProgram,1000);
        SendAddress(Address);
        for(int i =0;i<Length;i++){
        	uint8_t val = *(Data+i);
            SPIHandler::WriteData(this->Bus, val,1000);
            #if W25X40CL_DEBUG_MESSAGES == 1
                printf("0x%04X - %02d\n", Address+i, (int)val);
            #endif
        }
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::PageProgram;
        uint8_t *TxBuffer;
        int PacketLength = 4 + Length;
        TxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        /*
            0   - Command
            1   - Address (24 -> 16)
            2   - Address (15 -> 8)
            3   - Address (7 -> 0)
            4   - Data  
        */
        *TxBuffer = Program;
		#if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Command Byte:\n");
            printf("      0x%02X\n", (unsigned char)(*TxBuffer));
		#endif
        *(TxBuffer+1) = (Address >> 16) & 0xFF;
        *(TxBuffer+2) = (Address >> 8) & 0xFF;
        *(TxBuffer+3) = Address & 0xFF;
        #if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Addressing Bytes:\n");
            printf("      0xFF0000 -   0x%02X\n", (unsigned char)(*(TxBuffer+1)));
            printf("      0x00FF00 -   0x%02X\n", (unsigned char)(*(TxBuffer+2)));
            printf("      0x0000FF -   0x%02X\n", (unsigned char)(*(TxBuffer+3)));
        #endif
        #if W25X40CL_DEBUG_MESSAGES == 1
        	printf("   Transmitted Bytes:\n");
		#endif
        for(int i=0;i<Length;i++){
            uint8_t Var = *(Data+i);
        	*(TxBuffer+(4+i)) = Var;
            //*(Data+i) = Var;
			#if W25X40CL_DEBUG_MESSAGES == 1
            	printf("      %02d -   0x%02X\n", i, (unsigned char)Var);
			#endif
        }
        uint16_t CommunicationLength = sizeof(uint8_t) * PacketLength;
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle, TxBuffer, CommunicationLength,1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    //SendCommand(W25X40CLCommand::WriteDisable);
    this->DeviceFlags &= ~(W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY);
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Awaiting...\n");
	#endif
    while(IsReady() == false){HAL_Delay(1);}
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("IsReady!\n");
	#endif
}
void W25X40CL::SendAddress(uint32_t Address){
    uint8_t ShiftAddress = (Address >> 16) & 0xFF;
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::WriteData(this->Bus,ShiftAddress,1);
        ShiftAddress = (Address >> 8) & 0xFF;
        SPIHandler::WriteData(this->Bus,ShiftAddress,1);
        ShiftAddress = Address & 0xFF;
        SPIHandler::WriteData(this->Bus,ShiftAddress,1);
    #else
        uint8_t RxBuff = 0;
        HAL_SPI_TransmitReceive(SPIHandle,&ShiftAddress,&RxBuff,sizeof(uint8_t),1);
        ShiftAddress = (Address >> 8) & 0xFF;
        HAL_SPI_TransmitReceive(SPIHandle,&ShiftAddress,&RxBuff,sizeof(uint8_t),1);
        ShiftAddress = Address& 0xFF;
        HAL_SPI_TransmitReceive(SPIHandle,&ShiftAddress,&RxBuff,sizeof(uint8_t),1);
    #endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
}
void W25X40CL::ReadData(uint32_t Address, uint8_t *Data, uint16_t Length){
    if(Length == 0){return;}
    //if(Data != NULL){return;}
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    this->DeviceFlags |= W25X40CL_FLAG_ISBUSY;
    //Data = (uint8_t *)malloc(Length * sizeof(uint8_t));
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Reading Data...\n");
    #endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::Read,1);
        //SendAddress(Address);
        uint8_t TxNull = 0;
        for(int i =0;i<Length;i++){
        	//uint8_t val = SPIHandler::ReadData(this->Bus,100);
            uint8_t val;
            SPIHandler::Transfer(this->Bus, &TxNull, &val, sizeof(uint8_t));
            *(Data+i) = val;
            #if W25X40CL_DEBUG_MESSAGES == 1
                printf("0x%04X - %02d\n", Address+i, ((unsigned char)val));
            #endif
        }
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::Read;
        uint8_t *TxBuffer;
        uint8_t *RxBuffer;
        int PacketLength = 4 + Length;
        TxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        RxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        /*
            0   - Command
            1   - Address (24 -> 16)
            2   - Address (15 -> 8)
            3   - Address (7 -> 0)
            4   - Data  
        */
        *TxBuffer = Program;
		#if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Command Byte:\n");
            printf("      0x%02X\n", (unsigned char)(*TxBuffer));
		#endif
        *(TxBuffer+1) = (Address >> 16) & 0xFF;
        *(TxBuffer+2) = (Address >> 8) & 0xFF;
        *(TxBuffer+3) =  Address & 0xFF;
        #if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Addressing Bytes:\n");
            printf("      0xFF0000 -   0x%02X\n", (unsigned char)(*(TxBuffer+1)));
            printf("      0x00FF00 -   0x%02X\n", (unsigned char)(*(TxBuffer+2)));
            printf("      0x0000FF -   0x%02X\n", (unsigned char)(*(TxBuffer+3)));
        #endif
        uint16_t CommunicationLength = sizeof(uint8_t) * PacketLength;
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(SPIHandle, TxBuffer, RxBuffer,CommunicationLength,1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
		#if W25X40CL_DEBUG_MESSAGES == 1
        	printf("   Received Bytes:\n");
		#endif
        #if W25X40CL_DEBUG_MESSAGES == 1
            #if W25X40CL_DEBUG_BUFFERS == 1
                printf("   RX-Buffers:\n");
                printf("   =======================\n");
                printf("      Index     TX       RX\n");
                for(int i=0;i<CommunicationLength;i++){
                    printf("      %03d      %03d     %03d\n",i,(int)(*(TxBuffer+i)),(int)(*(RxBuffer+i)));
                }
                printf("   =======================\n\n");
            #endif
        #endif
        for(int i=0;i<Length;i++){
        	uint8_t var = *(RxBuffer+(4+i));
            *(Data+i) = var;
			#if W25X40CL_DEBUG_MESSAGES == 1
            	printf("      %02d -   0x%02X\n", i, (unsigned char)var);
			#endif
        }
        free(TxBuffer);
        free(RxBuffer);
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    this->DeviceFlags &= ~W25X40CL_FLAG_ISBUSY;
}

uint32_t W25X40CL::Write(uint32_t Address, bool Data){
    WriteData(Address, (uint8_t*)&Data, 1);
    return SIZE_BYTE;
}
uint32_t W25X40CL::Write(uint32_t Address, char Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: char, With data: %d\n", Data);
    #endif
    WriteData(Address, (uint8_t*)&Data, 1);
    return SIZE_BYTE;
}
uint32_t W25X40CL::Write(uint32_t Address, int Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: long, With data: %d\n", Data);
    #endif
    union {
        int True;
        uint8_t Bytes[2];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 2);
    return SIZE_WORD;
}
uint32_t W25X40CL::Write(uint32_t Address, long Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: long, With data: %ld\n", Data);
    #endif
    union {
        long True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 4);
    return SIZE_DWORD;
}
uint32_t W25X40CL::Write(uint32_t Address, String &Data){
    uint8_t Length = Data.length() + 1;
    uint8_t * TempString = new uint8_t[Length];
    Data.toCharArray((char *)TempString, Length);
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        //printf("Data type: string, With data: %s\n", &(char *)TempString);
    #endif
    WriteData(Address, &Length, 1);
    WriteData(Address + 1, TempString, Length);
    delete [] TempString;
    return (Length + 1) * SIZE_BITS;
}
uint32_t W25X40CL::Write(uint32_t Address, uint8_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: uint8_t, With data: %d\n", Data);
    #endif
    WriteData(Address, &Data, 1);
    return SIZE_BYTE;
}
uint32_t W25X40CL::Write(uint32_t Address, uint16_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: uint16_t, With data: %u\n", (unsigned int)Data);
    #endif
    union {
        uint16_t True;
        uint8_t Bytes[2];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 2);
    return SIZE_WORD;
}
uint32_t W25X40CL::Write(uint32_t Address, uint32_t Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: uint32_t, With data: %u\n", (unsigned long)Data);
    #endif
    union {
        uint32_t True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 4);
    return SIZE_DWORD;
}
uint32_t W25X40CL::Write(uint32_t Address, uint8_t * Data, uint8_t Length){
    if (Length == 0){return 0;}
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        printf("Data type: uint8_t, With data: ", Data);
        for(int i=0;i<Length;i++){
            printf("%d ", Data[i]);
        }
        printf("\n");
    #endif
    WriteData(Address, Data, Length);
    return SIZE_BYTE;
}
uint32_t W25X40CL::Write(uint32_t Address, float Data){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("W25X40CL - Data Write\n");
        //printf("Data type: float, With data: %f\n", Data);
    #endif
    union {
        float True;
        uint8_t Bytes[4];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 4);
    return SIZE_DWORD;
}
uint32_t W25X40CL::Write(uint32_t Address, double Data){
    union {
        double True;
        uint8_t Bytes[8];
    } Value;
    Value.True = Data;
    WriteData(Address, Value.Bytes, 8);
    return SIZE_QWORD;
}

uint32_t W25X40CL::Read(uint32_t Address, String* Output){
    uint8_t Length = 0;
    ReadData(Address, &Length, 1);
    uint8_t *Data = new uint8_t[Length];
    ReadData(Address + 1, Data, Length);
    *Output = "";
    for(uint8_t i=0;i<Length;i++){
        char Byte = *(Data + i);
        *Output += Byte;
    }
    delete []Data;
    return (Length +1) * SIZE_BITS;
}
uint32_t W25X40CL::Read(uint32_t Address, bool* Output){
    uint8_t Data = 0;
    ReadData(Address, &Data, 1);
    *Output = (bool)Data;
    return SIZE_BYTE;
}
uint32_t W25X40CL::Read(uint32_t Address, char* Output){
    uint8_t Data = 0;
    ReadData(Address, &Data, 1);
    *Output = (char)Data;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at %04X - %d\n", Address, Data);
    #endif
    return SIZE_BYTE;
}
uint32_t W25X40CL::Read(uint32_t Address, int* Output){
    union {
        uint32_t True;
        uint8_t Bytes[2];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 2);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - %d\n", Value.True);
    #endif
    return SIZE_WORD;
}
uint32_t W25X40CL::Read(uint32_t Address, long* Output){
    union {
        long True;
        uint8_t Bytes[4];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 4);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - %ld\n", Address, Value.True);
    #endif
    return SIZE_BYTE;
}
uint32_t W25X40CL::Read(uint32_t Address, uint8_t* Output){
    uint8_t Data = 0;
    ReadData(Address, &Data, 1);
    *Output = Data;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - %u\n", Address, (int)Data);
    #endif
    return SIZE_BYTE;
}
uint32_t W25X40CL::Read(uint32_t Address, uint16_t* Output){
    union {
        uint16_t True;
        uint8_t Bytes[2];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 2);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - %u\n", Address, (unsigned int)Value.True);
    #endif
    return SIZE_WORD;
}
uint32_t W25X40CL::Read(uint32_t Address, uint32_t* Output){
    union {
        uint32_t True;
        uint8_t Bytes[4];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 4);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - %lu\n", Address, (unsigned long)Value.True);
    #endif
    return SIZE_DWORD;
}
uint32_t W25X40CL::Read(uint32_t Address, uint8_t* Output, uint8_t Length){
    uint8_t Data = 0;
    ReadData(Address, &Data, Length);
    *Output = Data;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Data at 0x%04X - ", Address, (int)Data);
        for(int i=0;i<Length;i++){
            printf("%u ", Output[i]);
        }
        printf("\n");
    #endif
    return Length * SIZE_BITS;
}
uint32_t W25X40CL::Read(uint32_t Address, float* Output){
    union {
        float True;
        uint8_t Bytes[4];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 4);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        //printf("Data at %04X - %f\n", Value.True);
    #endif
    return SIZE_DWORD;
}
uint32_t W25X40CL::Read(uint32_t Address, double* Output){
    union {
        double True;
        uint8_t Bytes[8];
    } Value = {.True = 0 };
    ReadData(Address, Value.Bytes, 8);
    *Output = Value.True;
    #if W25X40CL_DEBUG_MESSAGES == 1
        //printf("Data at %04X - %f\n", Value.True);
    #endif
    return SIZE_QWORD;
}

void W25X40CL::EraseSector(uint32_t Address){
    SendCommand(W25X40CLCommand::WriteEnable);
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    this->DeviceFlags |= W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY;
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::SectorErase, 100);
        SendAddress(Address);
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::SectorErase;
        uint8_t *TxBuffer;
        int PacketLength = 4;
        TxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        /*
            0   - Command
            1   - Address (24 -> 16)
            2   - Address (15 -> 8)
            3   - Address (7 -> 0)
        */
        *TxBuffer = Program;
		#if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Command Byte:\n");
            printf("      0x%02X\n", (unsigned char)(*TxBuffer));
		#endif
        *(TxBuffer+1) = (Address >> 16) & 0xFF;
        *(TxBuffer+2) = (Address >> 8) & 0xFF;
        *(TxBuffer+3) = Address & 0xFF;
        #if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Addressing Bytes:\n");
            printf("      0xFF0000 -   0x%02X\n", (unsigned char)(*(TxBuffer+1)));
            printf("      0x00FF00 -   0x%02X\n", (unsigned char)(*(TxBuffer+2)));
            printf("      0x0000FF -   0x%02X\n", (unsigned char)(*(TxBuffer+3)));
        #endif
        uint16_t CommunicationLength = sizeof(uint8_t) * PacketLength;
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle, TxBuffer, CommunicationLength,1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    #endif 
    SendCommand(W25X40CLCommand::WriteDisable);
    this->DeviceFlags &= ~(W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY);
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Awaiting...\n");
	#endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    while(IsReady() == false){HAL_Delay(1);}
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("IsReady!\n");
	#endif
}
void W25X40CL::EraseBlock(uint32_t Address){
    SendCommand(W25X40CLCommand::WriteEnable);
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    this->DeviceFlags |= W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY;
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::BlockErase64kB, 100);
        SendAddress(Address);
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::BlockErase64kB;
		HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle,&Program,sizeof(uint8_t),1);
        SendAddress(Address);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    SendCommand(W25X40CLCommand::WriteDisable);
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    this->DeviceFlags &= ~(W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY);
}
void W25X40CL::EraseAll(){
    SendCommand(W25X40CLCommand::WriteEnable);
    if((this->DeviceFlags & W25X40CL_FLAG_ISBUSY)==W25X40CL_FLAG_ISBUSY){return;}
    this->DeviceFlags |= W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Erasing all memory...\n");
    #endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::EraseAll, 100);
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::BlockErase64kB;
		HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle,&Program,sizeof(uint8_t),1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    SendCommand(W25X40CLCommand::WriteDisable);
    this->DeviceFlags &= ~(W25X40CL_FLAG_WRITE_ENABLED | W25X40CL_FLAG_ISBUSY);
}

void W25X40CL::WriteProtect(bool Enable){
    if(Enable == true){
        HAL_GPIO_WritePin(this->WPPort, this->WPPin, GPIO_PIN_RESET);
        this->DeviceFlags |= W25X40CL_WRITE_PROTECTED;
    }
    else{
        HAL_GPIO_WritePin(this->WPPort, this->WPPin, GPIO_PIN_SET);
        this->DeviceFlags &= ~W25X40CL_WRITE_PROTECTED;
    }
}

bool W25X40CL::IsReady(){
    if((ReadState()&0x01) == 0x01){return false;}
    return true;
}
uint8_t W25X40CL::ReadState(void){
    uint8_t RxBuffer = 0;
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("Checking State...\n");
	#endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::ReadStatusRegister,1);
        RxBuffer = SPIHandler::ReadData(this->Bus,1);
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::ReadStatusRegister;
        uint8_t TxNull = Program;
        
        //int PacketLength = 2;
		#if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Command Byte:\n");
            printf("      0x%02X\n", (unsigned char)Program);
		#endif
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(SPIHandle, &Program, sizeof(uint8_t),100);
        HAL_SPI_TransmitReceive(SPIHandle, &TxNull, &RxBuffer,sizeof(uint8_t),1);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
		#if W25X40CL_DEBUG_MESSAGES == 1
        	printf("   Received Bytes:\n");
            printf("      0x%02X\n", (unsigned char)RxBuffer);
		#endif
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    return RxBuffer;
}

void W25X40CL::ReadJEDECID(void){
    #if W25X40CL_DEBUG_MESSAGES == 1
        printf("JEDEC ID...\n");
	#endif
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
    #if W25X40CL_USE_SPIHANDLER == 1
        SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
        SPIHandler::WriteData(this->Bus,(uint8_t)W25X40CLCommand::ReadJID,100);
        uint8_t TxNull = 0;
        for(int i =0;i<3;i++){
            uint8_t var;
            SPIHandler::Transfer(this->Bus, &TxNull, &var, sizeof(uint8_t));
            //*(Data+i) = val;
            #if W25X40CL_DEBUG_MESSAGES == 1
            	printf("      %02d -   0x%02X\n", i, (unsigned char)var);
			#endif
        }
        SPIHandler::EndTransaction(this->Bus);
    #else
        uint8_t Program = (uint8_t)W25X40CLCommand::ReadJID;
        uint8_t *TxBuffer;
        uint8_t *RxBuffer;
        int PacketLength = 4;
        TxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        RxBuffer = (uint8_t*)calloc(PacketLength, sizeof(uint8_t));
        /*
            0   - Command
            4   - Data  
        */
        *TxBuffer = Program;
		#if W25X40CL_DEBUG_MESSAGES == 1
            printf("   Command Byte:\n");
            printf("      0x%02X\n", (unsigned char)(*TxBuffer));
		#endif
        uint16_t CommunicationLength = sizeof(uint8_t) * PacketLength;
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(SPIHandle, TxBuffer, RxBuffer,CommunicationLength,100);
        HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
		#if W25X40CL_DEBUG_MESSAGES == 1
        	printf("   Received Bytes:\n");
		#endif
		#if W25X40CL_DEBUG_MESSAGES == 1
        for(int i=0;i<3;i++){
        	uint8_t var = *(RxBuffer+(1+i));
            printf("      %d -   0x%02X\n", i, (unsigned char)var);
        }
		#endif
        free(TxBuffer);
        free(RxBuffer);
    #endif 
    #if W25X40CL_USE_WATCH_DOG ==1
		RefreshWatchDog();
	#endif
}


uint32_t W25X40CL::DeriveAddress(uint8_t Block, uint8_t Sector, uint16_t Address){
    
    //--BBSXXX
    //--543210
    /*
        16 Sectors (4kB) per Block (64kB), 
        Block 0 - 7 (64kB){
            Sector 0 - 15 (4kB){
                Address 0 - 4095
            }
        }
    
    */
    uint32_t TempAddress = (Block << 16) | ((Sector & 0x0F)<< 12) | (Address & 0xFFF);
    return TempAddress;
}

uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, bool SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, char SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, int SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, long SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}

uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, float SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, double SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}

uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, uint8_t SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, uint16_t SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}
uint32_t W25X40CL::Append(uint8_t * Destination, uint32_t Length, uint32_t SourceData){
    uint32_t FullSize = Length + sizeof(SourceData);
    //memmove(Destination + Length, &SourceData, FullSize);
    uint8_t * temp = (uint8_t *)realloc(Destination, (FullSize + 1) *sizeof(uint8_t));
    Destination = temp;
    return FullSize;
}

uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, bool * Out){
    uint8_t Temp = 0;
    uint32_t Len = Extract(Source, Length, Start, &Temp);
    *Out = (bool)Temp;
    return Len;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, char * Out){
    uint8_t Temp = 0;
    uint32_t Len = Extract(Source, Length, Start, &Temp);
    *Out = (char)Temp;
    return Len;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, int * Out){
    uint16_t Temp = 0;
    uint32_t Len = Extract(Source, Length, Start, &Temp);
    *Out = (int)Temp;
    return Len;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, long * Out){
    uint32_t Temp = 0;
    uint32_t Len = Extract(Source, Length, Start, &Temp);
    *Out = (long)Temp;
    return Len;
}

uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, float * Out){
    union {
        float Output;
        uint32_t True;
    } Value;
    uint32_t Len = Extract(Source, Length, Start, &Value.True);
    *Out = Value.Output;
    return Len;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, double * Out){
    const int ArryLen = 8;
    if (Source == NULL){return 0.0f;}
    if (Start+ArryLen>Length){return 0.0f;}
    union {
        double True;
        uint8_t Bytes[ArryLen];
    } Value;
    for(int i=0;i<ArryLen;i++){
        Value.Bytes[i] = Source[Start + i];
    }
    *Out = Value.True;
    return ArryLen;
}

uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint8_t * Out){
    if (Source == NULL){return 0;}
    if (Start>Length){return 0;}
    *Out = Source[Start];
    return 1;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint16_t * Out){
    const int ArryLen = 2;
    if (Source == NULL){return 0;}
    if (Start+ArryLen>Length){return 0;}
    union {
        uint32_t True;
        uint8_t Bytes[ArryLen];
    } Value;
    for(int i=0;i<ArryLen;i++){
        Value.Bytes[i] = Source[Start + i];
    }
    *Out = Value.True;
    return ArryLen;
}
uint32_t W25X40CL::Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint32_t * Out){
    const int ArryLen = 4;
    if (Source == NULL){return 0;}
    if (Start+ArryLen>Length){return 0;}
    union {
        uint32_t True;
        uint8_t Bytes[ArryLen];
    } Value;
    for(int i=0;i<ArryLen;i++){
        Value.Bytes[i] = Source[Start + i];
    }
    *Out = Value.True;
    return ArryLen;
}
#if W25X40CL_USE_WATCH_DOG ==1
	void W25X40CL::RefreshWatchDog(){
		if (hnl_watchdog == NULL){ return; }
		HAL_IWDG_Refresh(hnl_watchdog);
	}
	void W25X40CL::LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog){
		hnl_watchdog = HandleWatchDog;
		RefreshWatchDog();
	}
#endif

