#include "ModbusRTU.h"
#if MODBUS_RTU_DEBUG_MESSAGES == 1
    #include <stdio.h>
#endif
/*
    MODBUS: https://modbus.org/docs/Modbus_over_serial_line_V1.pdf

    FRAME INFORMATION:
        START:  >3.5 (4)
        ADDRS:  1
        FUNCT:  1
        DATA:   8n
        CRC:    2
        STOP:   >3.5 (4)
*/

//HAL_UART_Transmit(handle, (uint8_t *)&TempString, Length, HAL_MAX_DELAY);


ModbusRTU::ModbusRTU(UART_HandleTypeDef * hndlComms, uint8_t * CBuffer, TIM_HandleTypeDef * hndlTmr, GPIO_TypeDef * hndlEna, int16_t pinEna){
    HndlComms = hndlComms;
    HndlTmr35 = hndlTmr;
    HndlPortEnable = hndlEna;
    PinEnable = pinEna;
    IsMaster = true;
    CommBuff = CBuffer;
    //if (CommBuff == NULL){
        HAL_UART_Receive_IT(hndlComms, CommBuff, 1);
    //}
    HAL_TIM_Base_Start_IT(hndlTmr);
}
ModbusRTU::ModbusRTU(UART_HandleTypeDef * hndlComms, uint8_t * CBuffer, TIM_HandleTypeDef * hndlTmr, GPIO_TypeDef * hndlEna, int16_t pinEna, uint8_t slaveAddress){
    HndlComms = hndlComms;
    HndlTmr35 = hndlTmr;
    HndlPortEnable = hndlEna;
    PinEnable = pinEna;
    IsMaster = false;
    SlaveAddress = slaveAddress;
    CommBuff = CBuffer;
    //if (CommBuff == NULL){
        HAL_UART_Receive_IT(hndlComms, CommBuff, 1);
    //}
    HAL_TIM_Base_Start_IT(hndlTmr);
}

volatile uint32_t * ModbusRTU::LinkSilenceTick(){
	//HAL_TIM_Base_Start_IT(HndlTmr35);
    return &this->TickSilence;
}
ModbusError ModbusRTU::InvokeTransmit(uint8_t SlaveAddress, ModbusFunction Function, uint8_t * Frame, uint16_t Length){
    ModbusError eStatus = MB_ENOERR;
    //__disable_irq();
    #if MODBUS_RTU_DEBUG_MESSAGES == 1
        printf("MODBUS RTU: Invoking Transmission\n");
    #endif
    if(RXState == MBUS_RX_IDLE){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("----------------------------------\n");
            printf("MODBUS RTU: TX - Adr:%03d, Func:%03d\n", (int)SlaveAddress, (int)Function);
        #endif
        /*
            ADU
                    ADR     1 byte
            PDU     
                    FUN     1 byte
            CRC     
                    CRC     2 bytes

        */
        BytesToSend = 2;
        TXBuffer[MODBUS_RTU_ADU_OFFSET] = SlaveAddress;
        TXBuffer[MODBUS_RTU_PDU_OFFSET] = (uint8_t)Function;
        BytesToSend += Length;
        uint8_t * DataBuffer = (uint8_t *)malloc(2 + Length);
        DataBuffer[0] = TXBuffer[MODBUS_RTU_ADU_OFFSET];
        DataBuffer[1] = TXBuffer[MODBUS_RTU_PDU_OFFSET];
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
                //  ------------
            printf("            TX - Data\n");
        #endif
        for(int i=0;i<Length;i++){
            DataBuffer[MODBUS_RTU_DATA+i] = Frame[i];
            TXBuffer[MODBUS_RTU_DATA + i] = Frame[i];
            #if MODBUS_RTU_DEBUG_MESSAGES == 1
                printf("            (%03d) = %03d\n", i, (int)Frame[i]);
            #endif
        }
        uint16_t CRC16 = Modbus::CalculateCRC16(DataBuffer, BytesToSend);
        uint8_t CRCHigh = (uint8_t)(CRC16 & 0xFF);
        uint8_t CRCLow = (uint8_t)(CRC16 >> 8);
        TXBuffer[BytesToSend++] = CRCHigh;
        TXBuffer[BytesToSend++] = CRCLow;
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("            TX - CRC: %03d, %03d\n", (int)CRCHigh, (int)CRCLow);
            printf("----------------------------------\n");
        #endif
        TXState = MBUS_TX_READY;
        free(DataBuffer);
        //CommunicationsInterrupts(false, true);
    }
    else{
        eStatus = MB_EIO;
    }
    //__enable_irq();
    return eStatus;
}
void ModbusRTU::Transmit(){
    if (HndlComms == NULL){return;}
    if (HndlPortEnable == NULL){return ;}
    if (BytesToSend > 0){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: TX - Transmitting...\n");
        #endif
        TXState = MBUS_TX_PROCESS;
        #if MODBUS_USE_WATCH_DOG ==1
            RefreshWatchDog();
        #endif
        HAL_GPIO_WritePin(HndlPortEnable, PinEnable, GPIO_PIN_SET);
        HAL_UART_Transmit(HndlComms, (uint8_t*)TXBuffer, BytesToSend, 10);
        HAL_GPIO_WritePin(HndlPortEnable, PinEnable, GPIO_PIN_RESET);
        #if MODBUS_USE_WATCH_DOG ==1
            RefreshWatchDog();
        #endif
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: TX - Done\n");
        #endif
        ResetSilenceCounter();
    }
    TXState = MBUS_TX_EMITTED;
    TickLastSent = HAL_GetTick();
}
/*
    STATE LOGIC (MASTER)
    -----------------------------------------------------------------------

    Action:                 TXDA    

    System State:   IDLE    FrSent  

    RX State:       IDLE    IDLE    IDLE    IDLE

    TX State:       IDLE    REDY    PROC    EMIT

*/
void ModbusRTU::Poll(){
    if (IsMaster == true){ PollMaster();}
    else{ PollSlave();}
    if (CheckState(&LastReceivedTick,5000) ==  true){
        HAL_UART_Receive_IT(HndlComms, CommBuff, 1);
    }
}
void ModbusRTU::PollMaster(){
    switch(SystemState){
        case MBUS_SYS_IDLE:
            if (TXState == MBUS_TX_READY){
                SetState(MBUS_SYS_TRANSMIT);
            }
            else if (TXState == MBUS_TX_EMITTED){
                if ((HAL_GetTick() - TickLastSent) >= MODBUS_RTU_TIMEOUT){ 
                    TXState = MBUS_TX_IDLE;
                    RXState = MBUS_RX_IDLE;
                    ClearRXBuffer(this);
                    #if MODBUS_RTU_DEBUG_MESSAGES == 1
                        printf("MODBUS RTU: TX - System going into idle mode\n");
                    #endif
                }
            }
            else{RXState = MBUS_RX_IDLE;}
            break;
        case MBUS_SYS_RECEIVED_BYTE:
            if(TickSilence >= MODBUS_RTU_MIN_TICK){
                if(BytesToReceive > 0){
                    //SetState(MBUS_SYS_RECEIVED_FRAME);
                    ProcessFrame();
                    ClearRXBuffer(this);
                    SetState(MBUS_SYS_IDLE);
                    RXState = MBUS_RX_IDLE;
                    TXState = MBUS_TX_IDLE;
                }
                //else{
                //    SetState(MBUS_SYS_IDLE);
                //    RXState = MBUS_RX_IDLE;
                //}
            }
            if ((HAL_GetTick() - TickLastSent) >= MODBUS_RTU_TIMEOUT){ 
                TXState = MBUS_TX_IDLE;
                #if MODBUS_RTU_DEBUG_MESSAGES == 1
                    printf("MODBUS RTU: TX - Receiving timeout, going into idle mode\n");
                #endif
                SetState(MBUS_SYS_IDLE);
                RXState = MBUS_RX_IDLE;
            }
            break;
        case MBUS_SYS_RECEIVED_FRAME:
            ProcessFrame();
            ClearRXBuffer(this);
            SetState(MBUS_SYS_IDLE);
            break;
        case MBUS_SYS_TRANSMIT:
            if(TXState == MBUS_TX_READY){ Transmit();}
            else{
                SetState(MBUS_SYS_RECEIVED_BYTE);
            }
            break;
            
        default:
            break;
    }
    if(LatchedSystemState != SystemState){
        LatchedSystemState = SystemState;
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            PrintState();
        #endif
    }
}
#if MODBUS_RTU_DEBUG_MESSAGES == 1
    void ModbusRTU::PrintState(){
        switch(SystemState){
            case MBUS_SYS_IDLE: printf("MODBUS RTU: Idling...\n"); break;
            case MBUS_SYS_RECEIVED_BYTE: printf("MODBUS RTU: Receiving\n"); break;
            case MBUS_SYS_RECEIVED_FRAME: printf("MODBUS RTU: Frame Reception Complete\n"); break;
            case MBUS_SYS_TRANSMIT: printf("MODBUS RTU: Transmitting...\n"); break;
            default:
                break;
        }
    }
#endif 
void ModbusRTU::PollSlave(){
    switch(SystemState){
        case MBUS_SYS_RECEIVED_BYTE:
            //Check to see if 3.5 bytes have elapsed
            if(TickSilence >= MODBUS_RTU_MIN_TICK){
                SystemState = MBUS_SYS_RECEIVED_FRAME;
            }
            break;
        case MBUS_SYS_RECEIVED_FRAME:
            //Full or part frame has been received, go and process frame
            ProcessFrame();
            ClearRXBuffer(this);
            SystemState = MBUS_SYS_IDLE;
            break;
        case MBUS_SYS_FRAME_SENT:
            if(TickSilence >= MODBUS_RTU_MIN_TICK){
                SystemState = MBUS_SYS_RECEIVED_FRAME;
            }
        default:
            break;
    }
}

void ModbusRTU::SetState(ModbusState State){
    PreviousSystemState = SystemState;
    SystemState = State;
}
//Appends Received Byte to Received Buffer
void ModbusRTU::AppendByte(uint8_t Input){
    if (SystemState <= MBUS_SYS_RECEIVED_BYTE){
        if (BytesToReceive < MODBUS_RTU_RXBUFFER_MAX - 1){
            ResetSilenceCounter();
            RXBuffer[BytesToReceive] = Input;
            BytesToReceive++;
            RXState = MBUS_RX_RCV;
            //#if MODBUS_RTU_DEBUG_MESSAGES == 1
            //    printf("            RX - DATA: %03d\n", (int)RXBuffer[BytesToReceive-1]);
            //#endif
            ResetSilenceCounter();
        }
        else{RXState = MBUS_RX_ERROR;}
    }
}

//Processes Received Frame
void ModbusRTU::ProcessFrame(){
    #if MODBUS_RTU_DEBUG_MESSAGES == 1
        printf("MODBUS RTU: Processing frame...\n");
        printf("----------------------------------\n");
        printf("MODBUS RTU: RX - Len:%03d\n", BytesToReceive);

    #endif
    uint8_t Address = RXBuffer[MODBUS_RTU_ADDRESS];
    uint8_t Function = RXBuffer[MODBUS_RTU_FUNCTION];
    uint16_t Length = BytesToReceive;
    #if MODBUS_RTU_DEBUG_MESSAGES == 1  
        printf("            RX - Adr:%03d, Func:%03d\n", Address, Function);
    #endif
    if (IsMaster == false){
        if (Address != SlaveAddress){
            //Return Error!
            #if MODBUS_RTU_DEBUG_MESSAGES == 1  
                printf("            RX - Adr:Incorrect\n");
                printf("----------------------------------\n");
            #endif
            return;
        }
    }
    bool CRCState = VerifyRXCRC(this);

    #if MODBUS_RTU_DEBUG_MESSAGES == 1  
        if(CRCState==false){
            printf("            RX - CRC:Incorrect!\n");
        }
        else{
            printf("            RX - CRC:Okay\n");
        }
        printf("            RX - Data Stream: ");
        for(int i=0;i<Length;i++){
            printf("%03d ", RXBuffer[i]);
        }
        printf("\n");
        printf("----------------------------------\n");
    #endif
    if(CRCState==false){return;}
    uint8_t StartLength = MODBUS_RTU_DATA + 1;
    uint8_t EndLength = Length - 2;
    if(IsMaster == true){
        if ((Function == MBUS_FUNC_ReadCoils)||(Function == MBUS_FUNC_ReadDiscreteInputs)){
            ModbusCoil ReturnedCoil;
            bool Result = GenerateCoilStruct(Length, StartLength, EndLength, &ReturnedCoil);
            if (Result == true){
                if (OnReadCoilsReturn != NULL){ OnReadCoilsReturn(this, &ReturnedCoil);}
            }
            CleanUp(&ReturnedCoil);
        }
        else if ((Function == MBUS_FUNC_ReadHoldingRegisters)||(Function == MBUS_FUNC_ReadInputRegisters)){
            ModbusRegister ReturnedRegister;
            bool Result = GenerateRegisterStruct(Length, StartLength, EndLength, &ReturnedRegister);
            if (Result == true){
                if (OnReadRegisterReturn != NULL){ OnReadRegisterReturn(this, &ReturnedRegister);}
            }
            CleanUp(&ReturnedRegister);
        }
        else if (Function == MBUS_FUNC_WriteSingleCoil){
            ModbusCoil ReturnedCoil;
            bool Result = GenerateResponseCoilWriteStruct(Length, StartLength, EndLength, &ReturnedCoil);
            if (Result == true){
                if (OnWriteCoilReturn != NULL){ OnWriteCoilReturn(this, &ReturnedCoil);}
            }
            CleanUp(&ReturnedCoil);
        }
        else if (Function == MBUS_FUNC_WriteSingleHoldingRegister){
            ModbusRegister ReturnedRegister;
            bool Result = GenerateResponseRegisterWriteStruct(Length, StartLength, EndLength, &ReturnedRegister);
            if (Result == true){
                if (OnWriteRegisterReturn != NULL){ OnWriteRegisterReturn(this, &ReturnedRegister);}
            }
            CleanUp(&ReturnedRegister);
        }
        else if ((Function & 0x80) == 0x80){
            ModbusFunction Func = (ModbusFunction)(Function & 0x7F);
            ModbusExceptionEvent Exception;
            Exception.RaisedAt = Func;
            Exception.Exception = (ModbusException)RXBuffer[MODBUS_RTU_DATA];
            if (OnExceptionRaised != NULL){ OnExceptionRaised(this, &Exception);}
        }
    }
}
/*
        Write Return

        ADR: 1byte - 0x02
        FUN: 1byte - 0x05
        REG: 2byte - 0x0000 - 0xFFFF
        DAT: 2byte - 0x0000 = 0, 0xFF00 = 1
*/

bool ModbusRTU::GenerateCoilStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusCoil * Coils){
    uint8_t BytesToRead = 0;
    uint8_t DiffChk = End - Start;
    if (Length >= 6){
        BytesToRead = RXBuffer[MODBUS_RTU_DATA];
    }
    Coils->Coils = (bool *)malloc(sizeof(bool)*BytesToRead);
    Coils->Count = DiffChk * 8;
    if (End<Start){return false;}
    if (BytesToRead != DiffChk){return false;}
    for(int i=Start;i<End;i++){
    //for(int i=End - 1;i>=Start;i--){
        uint8_t Byte = RXBuffer[i];
        for(int j=0;j<8;j++){
            uint8_t Shift = 0x01 << j;
            Coils->Coils[j] = (Shift & Byte) == Shift ? true : false;
        }
    }
    return true;
}
bool ModbusRTU::GenerateRegisterStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusRegister * Registers){
    uint8_t BytesToRead = 0;
    uint8_t DiffChk = End - Start;
    if (Length >= 7){
        BytesToRead = RXBuffer[MODBUS_RTU_DATA] / 2;
    }
    Registers->Registers = (uint16_t *)malloc(sizeof(uint16_t)*BytesToRead);
    Registers->Count = BytesToRead;
    if (End<Start){return false;}
    if ((DiffChk/2)!=BytesToRead){return false;}
    int j = 0;
    int k = 0;
    for(int i=Start;i<End;i++){
        if ((j % 0x02) == 0x00){
            uint16_t Word = (RXBuffer[i] << 0x08) + RXBuffer[i+1];
            Registers->Registers[k] = Word;
            ++k;
        }
        ++j;
    }
    return true;
}

//Start = MODBUS_RTU_DATA
bool ModbusRTU::GenerateResponseCoilWriteStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusCoil * Coil){
    uint8_t BytesToRead = 0;
    if (End<Start){return false;}
    BytesToRead = End + 1 - Start;
    uint16_t Address = 0x00;
    uint16_t Data = 0x00;
    if (BytesToRead == 4){
        Address = (RXBuffer[MODBUS_RTU_DATA] << 0x08) | RXBuffer[MODBUS_RTU_DATA + 1];
        Data = (RXBuffer[MODBUS_RTU_DATA + 2] << 0x08) | RXBuffer[MODBUS_RTU_DATA + 3];
    }
    else{return false;}
    Coil->Count =1;
    Coil->Coils = (bool *)malloc(sizeof(bool)*1);
    Coil->Coils[0] = Data == 0xFF00 ? true : false;
    return true;
}
bool ModbusRTU::GenerateResponseRegisterWriteStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusRegister * Register){
    uint8_t BytesToRead = 0;
    if (End<Start){return false;}
    BytesToRead = End + 1 - Start;
    uint16_t Address = 0x00;
    uint16_t Data = 0x00;
    if (BytesToRead == 4){
        Address = (RXBuffer[MODBUS_RTU_DATA] << 0x08) | RXBuffer[MODBUS_RTU_DATA + 1];
        Data = (RXBuffer[MODBUS_RTU_DATA + 2] << 0x08) | RXBuffer[MODBUS_RTU_DATA + 3];
    }
    else{return false;}
    Register->Count =1;
    Register->Registers = (uint16_t *)malloc(sizeof(uint16_t)*1);
    Register->Registers[0] = Data;
    return true;
}

void ModbusRTU::ClearRXBuffer(ModbusRTU * Controller){
    for(int i=0;i<MODBUS_RTU_RXBUFFER_MAX;i++){
        Controller->RXBuffer[i] = 0x00;
    }
    Controller->BytesToReceive = 0;
}
bool ModbusRTU::VerifyRXCRC(ModbusRTU * Controller){
    /*
        ADU
                ADR     1 byte
        PDU     
                FUN     1 byte
        CRC     
                CRC     2 bytes

    */
	bool CRCResult = false;
    uint8_t PDULength = Controller->BytesToReceive;
    uint8_t * Buffer = (uint8_t*)malloc(PDULength);
    for(int i=0;i<PDULength;i++){
        Buffer[i] = Controller->RXBuffer[i];
    }
    if(Modbus::CalculateCRC16(Buffer, PDULength) == 0){CRCResult = true;}
    free(Buffer);
    return CRCResult;
}

void ModbusRTU::ResetSilenceCounter(){
    TickSilence = 0;
}


void ModbusRTU::ReadCoils(uint8_t Address, uint16_t Start, uint16_t Count){
    if ((Count==0x00)||(Count>0x7D0)){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: RX - Count out of bounds: [1-2000]\n");
        #endif
        return;
    }
    SetLastAddress(Address, Start);
    ExecuteMasterFunction(MBUS_FUNC_ReadCoils, Address, Start, Count);
}
void ModbusRTU::ReadDiscreteInputs(uint8_t Address, uint16_t Start, uint16_t Count){
    if ((Count==0x00)||(Count>0x7D0)){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: RX - Count out of bounds: [1-125]\n");
        #endif
        return;
    }
    SetLastAddress(Address, Start);
    ExecuteMasterFunction(MBUS_FUNC_ReadDiscreteInputs, Address, Start, Count);
}
void ModbusRTU::ReadHoldingRegisters(uint8_t Address, uint16_t Start, uint16_t Count){
    if ((Count==0x00)||(Count>0x7D)){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: RX - Count out of bounds: [1-125]\n");
        #endif
        return;
    }
    SetLastAddress(Address, Start);
    ExecuteMasterFunction(MBUS_FUNC_ReadHoldingRegisters, Address, Start, Count);
}
void ModbusRTU::ReadInputRegisters(uint8_t Address, uint16_t Start, uint16_t Count){
    if ((Count==0x00)||(Count>0x7D)){
        #if MODBUS_RTU_DEBUG_MESSAGES == 1
            printf("MODBUS RTU: RX - Count out of bounds: [1-125]\n");
        #endif
        return;
    }
    SetLastAddress(Address, Start);
    ExecuteMasterFunction(MBUS_FUNC_ReadInputRegisters, Address, Start, Count);
}
void ModbusRTU::WriteSingleCoil(uint8_t Address, uint16_t Start, bool Value){
    if(IsMaster==false){return;}
    uint8_t Buffer[4];
    Buffer[0] = (uint8_t)((Start >> 8) & 0xFF);
    Buffer[1] = (uint8_t)(Start & 0xFF);
    Buffer[2] = Value == true ? 0xFF : 0x00;
    Buffer[3] = 0x00;
    SetLastAddress(Address, Start);
    InvokeTransmit(Address, MBUS_FUNC_WriteSingleCoil, Buffer, 4);
}
void ModbusRTU::WriteSingleRegister(uint8_t Address, uint16_t Start, uint16_t Value){
    if(IsMaster==false){return;}
    uint8_t Buffer[4];
    Buffer[0] = (uint8_t)((Start >> 8) & 0xFF);
    Buffer[1] = (uint8_t)(Start & 0xFF);
    Buffer[2] = (uint8_t)((Value >> 8) & 0xFF);
    Buffer[3] = (uint8_t)(Value & 0xFF);
    SetLastAddress(Address, Start);
    InvokeTransmit(Address, MBUS_FUNC_WriteSingleHoldingRegister, Buffer, 4);
}
void ModbusRTU::WriteMultipleRegisters(uint8_t Address, uint16_t Start, uint16_t Count, uint16_t * Data){
    if(IsMaster==false){return;}
    if (Count == 0){return;}
    if (Count > 0x7B){return;}
    uint8_t Length = Count + 5;
    uint8_t * Buffer = (uint8_t *)malloc(Length * sizeof(uint8_t));
    Buffer[0] = (uint8_t)((Start >> 8) & 0xFF);
    Buffer[1] = (uint8_t)(Start & 0xFF);
    Buffer[2] = (uint8_t)((Count >> 8) & 0xFF);
    Buffer[3] = (uint8_t)(Count & 0xFF);
    Buffer[3] = Count * 2;
    for(int i=0;i<Count;i++){
       Buffer[4+i] =  Data[i];
    }
    SetLastAddress(Address, Start);
    InvokeTransmit(Address, MBUS_FUNC_WriteMultipleHoldingRegisters, Buffer, Length);
    free(Buffer);
}


void ModbusRTU::ExecuteMasterFunction(ModbusFunction Function, uint8_t Address, uint16_t Start, uint16_t Count){
    if(IsMaster==false){return;}
    uint8_t Buffer[4];
    Buffer[0] = (uint8_t)((Start >> 8) & 0xFF);
    Buffer[1] = (uint8_t)(Start & 0xFF);
    Buffer[2] = (uint8_t)((Count >> 8) & 0xFF);
    Buffer[3] = (uint8_t)(Count & 0xFF);
    InvokeTransmit(Address, Function, Buffer, 4);
}

void ModbusRTU::AttachOnReadCoilsReturn(void (*OnReadCoilsReturn)(ModbusRTU *, ModbusCoil *)){
    this->OnReadCoilsReturn = OnReadCoilsReturn;
}
void ModbusRTU::AttachOnReadRegisterReturn(void (*OnReadRegisterReturn)(ModbusRTU *, ModbusRegister *)){
    this->OnReadRegisterReturn = OnReadRegisterReturn;
}

void ModbusRTU::AttachOnWriteCoilReturn(void (*OnWriteCoilReturn)(ModbusRTU *, ModbusCoil *)){
    this->OnWriteCoilReturn = OnWriteCoilReturn;
}
void ModbusRTU::AttachOnWriteRegisterReturn(void (*OnWriteRegisterReturn)(ModbusRTU *, ModbusRegister *)){
    this->OnWriteRegisterReturn = OnWriteRegisterReturn;
}
void ModbusRTU::AttachOnExceptionRaised(void (*OnExceptionRaised)(ModbusRTU *, ModbusExceptionEvent *)){
    this->OnExceptionRaised = OnExceptionRaised;
}

bool ModbusRTU::IsAddressEqual(uint8_t Address, uint16_t Start){
    if (Address == LastDeviceAddress){
        if (Start == LastRegister){ return true;}
    }
    return false;
}

void ModbusRTU::SetLastAddress(uint8_t Address, uint16_t Start){
    LastDeviceAddress = Address;
    LastRegister = Start;
}

bool ModbusRTU::CheckState(uint32_t *LastTick, uint32_t TickDuration){
    uint32_t CurrentTick = HAL_GetTick();
    if ((CurrentTick-(*LastTick))>TickDuration){
        *LastTick = CurrentTick;
        return true;
    }
    else{
        if (*LastTick > CurrentTick){*LastTick = CurrentTick;}
    }
    return false;
}

void ModbusRTU::CleanUp(ModbusRegister * Input){
    free(Input->Registers);
    Input->Count = 0;
}
void ModbusRTU::CleanUp(ModbusCoil * Input){
    free(Input->Coils);
    Input->Count = 0;
}

#if MODBUS_USE_WATCH_DOG ==1
	void ModbusRTU::RefreshWatchDog(){
		if (hnl_watchdog == NULL){ return; }
		HAL_IWDG_Refresh(hnl_watchdog);
	}
	void ModbusRTU::LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog){
		hnl_watchdog = HandleWatchDog;
		RefreshWatchDog();
	}
#endif