#include "ModbusRTU.h"


bool ModbusRTU::InitaliseTimer(uint16_t Timeout50us){
    TIM_MasterConfigTypeDef sMasterConfig;
    
    hndl_timer->Init.Prescaler = (HAL_RCC_GetPCLK1Freq() / 1000000) - 1;
    hndl_timer->Init.CounterMode = TIM_COUNTERMODE_UP;
    hndl_timer->Init.Period = 50 - 1;
    
    Timeout = Timeout50us;
    
    if (HAL_TIM_Base_Init(hndl_timer) != HAL_OK){
        return false;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(hndl_timer, &sMasterConfig) != HAL_OK){
        return false;
    }
    return true;
}
void ModbusRTU::EnableTimer(){ 
    DownCounter = Timeout;
    HAL_TIM_Base_Start_IT(hndl_timer);
}
void ModbusRTU::DisableTimer(){
    HAL_TIM_Base_Stop_IT(hndl_timer);
}

void ModbusRTU::CommunicationsInterrupts(bool RxIntEnable, bool TxIntEnable){
    if (RxIntEnable) {        
        __HAL_UART_ENABLE_IT(hndl_comms, UART_IT_RXNE);
    } 
    else {    
        __HAL_UART_DISABLE_IT(hndl_comms, UART_IT_RXNE);
    }
    if (TxIntEnable) {    
        __HAL_UART_ENABLE_IT(hndl_comms, UART_IT_TXE);
    } 
    else {
        __HAL_UART_DISABLE_IT(hndl_comms, UART_IT_TXE);
    }  
}
bool ModbusRTU::CommunicationsInitalise(uint8_t ucPORT, uint32_t ulBaudRate, uint8_t ucDataBits, ModbusParity eParity){
    return true;
}
bool ModbusRTU::TransmitByte(uint8_t Input){
    return (HAL_OK == HAL_UART_Transmit(hndl_comms, (uint8_t*)&Input, 1, 10));
}
bool ModbusRTU::ReceiveByte(uint8_t * Output){
    *Output = (uint8_t)(hndl_comms->Instance->DR & (uint8_t)0x00FF);  
    return true;
}

bool ModbusRTU::EventInitalise(){
    xEventInQueue = false;
    return true;
}
bool ModbusRTU::EventPost(ModbusState Event){
    xEventInQueue = true;
    eQueuedEvent = Event;
    return true;
}
bool ModbusRTU::EventRetrieve(ModbusState * Event){
    bool xEventHappened = false;
    if(xEventInQueue){
        *Event = eQueuedEvent;
        xEventInQueue = false;
        xEventHappened = true;
    }
    return xEventHappened;
}

ModbusError ModbusRTU::Initalise(uint8_t SlaveAddress, uint8_t Port, uint32_t BaudRate, ModbusParity Parity){
    ModbusError eStatus = MB_ENOERR;
    if((SlaveAddress == MB_ADDRESS_BROADCAST) || (SlaveAddress < MB_ADDRESS_MIN) || (SlaveAddress > MB_ADDRESS_MAX)){
        eStatus = MB_EINVAL;
    }
    else{
        ucMBAddress = SlaveAddress;

        //pvMBFrameStartCur = eMBRTUStart;
        //pvMBFrameStopCur = eMBRTUStop;
        //peMBFrameSendCur = eMBRTUSend;
        //peMBFrameReceiveCur = eMBRTUReceive;
        //pvMBFrameCloseCur = MB_PORT_HAS_CLOSE ? vMBPortClose : NULL;
        //pxMBFrameCBByteReceived = xMBRTUReceiveFSM;
        //pxMBFrameCBTransmitterEmpty = xMBRTUTransmitFSM;
        //pxMBPortCBTimerExpired = xMBRTUTimerT35Expired;

        eStatus = RTUInitalise(ucMBAddress, Port, BaudRate, Parity);

        if(eStatus == MB_ENOERR){
            if(!EventInitalise()){
                eStatus = MB_EPORTERR;
            }
            else{
                //eMBCurrentMode = eMode;
                SystemState = STATE_DISABLED;
            }
        }
    }
    return eStatus;
}
ModbusError ModbusRTU::Close(){
    ModbusError    eStatus = MB_ENOERR;
    if(SystemState == STATE_DISABLED){
        //if(pvMBFrameCloseCur != NULL){
        //    //pvMBFrameCloseCur();
        //}
    }
    else{
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}
ModbusError ModbusRTU::Enable(){
    ModbusError eStatus = MB_ENOERR;
    if(SystemState == STATE_DISABLED){
        RTUStart();
        SystemState = STATE_ENABLED;
    }
    else{
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}
ModbusError ModbusRTU::MasterDisable(){
    ModbusError eStatus;
    if((SystemState == STATE_ENABLED) || (SystemState == STATE_ESTABLISHED)){
        RTUStop();
        SystemState = STATE_DISABLED;
        eStatus = MB_ENOERR;
    }
    else if(SystemState == STATE_DISABLED){
        eStatus = MB_ENOERR;
    }
    else{
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

bool ModbusRTU::MasterIsEstablished(){
    if(SystemState == STATE_ESTABLISHED){
        return true;
    }
    else{
        return false;
    }
}

ModbusError ModbusRTU::Disable(){
    ModbusError    eStatus;
    if(SystemState == STATE_ENABLED){
        RTUStop();
        SystemState = STATE_DISABLED;
        eStatus = MB_ENOERR;
    }
    else if(SystemState == STATE_DISABLED){
        eStatus = MB_ENOERR;
    }
    else{
        eStatus = MB_EILLSTATE;
    }
    return eStatus;
}

ModbusError ModbusRTU::RTUInitalise(uint8_t SlaveAddress, uint8_t Port, uint32_t BaudRate, ModbusParity Parity){
    ModbusError eStatus = MB_ENOERR;
    uint32_t TimerT35_50us;

    (void)SlaveAddress;
    __disable_irq();
    if(CommunicationsInitalise(Port, BaudRate, 8, Parity) != true){
        eStatus = MB_EPORTERR;
    }
    else{
        if(BaudRate > 19200){
            TimerT35_50us = 35;       /* 1800us. */
        }
        else{
            TimerT35_50us = (7UL * 220000UL)/(2UL * BaudRate);
        }
        if(InitaliseTimer((uint16_t)TimerT35_50us) != true){
            eStatus = MB_EPORTERR;
        }
    }
    __enable_irq();
    return eStatus;
}

void ModbusRTU::RTUStart(){
    __disable_irq();
    RXState = STATE_RX_INIT;
    CommunicationsInterrupts(true, false);
    EnableTimer();
    __enable_irq();
}
void ModbusRTU::RTUStop(){
    __disable_irq();
    CommunicationsInterrupts(false, false);
    DisableTimer();
    __enable_irq();
}
ModbusError ModbusRTU::RTUReceive(uint8_t * pucRcvAddress, uint8_t ** pucFrame, uint16_t * pusLength ){
    bool xFrameReceived = false;
    ModbusError eStatus = MB_ENOERR;
	(void)(xFrameReceived);
    __disable_irq();
    assert_param( usRcvBufferPos < MB_SER_PDU_SIZE_MAX );
    if((usRcvBufferPos >= MB_SER_PDU_SIZE_MIN) && (Modbus::CalculateCRC16((uint8_t * ) ucRTUBuf, usRcvBufferPos ) == 0 )){
        *pucRcvAddress = ucRTUBuf[MB_SER_PDU_ADDR_OFF];
        *pusLength = (uint16_t)( usRcvBufferPos - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC );
        *pucFrame = (uint8_t *) & ucRTUBuf[MB_SER_PDU_PDU_OFF];
        xFrameReceived = true;
    }
    else{
        eStatus = MB_EIO;
    }
    __enable_irq();
    return eStatus;
}
ModbusError ModbusRTU::RTUSend(uint8_t ucSlaveAddress, const uint8_t * pucFrame, uint16_t usLength){
    ModbusError eStatus = MB_ENOERR;
    uint16_t usCRC16;

    __disable_irq();
    if(RXState == STATE_RX_IDLE){
        pucSndBufferCur = (uint8_t *)pucFrame - 1;
        usSndBufferCount = 1;
        pucSndBufferCur[MB_SER_PDU_ADDR_OFF] = ucSlaveAddress;
        usSndBufferCount += usLength;
        usCRC16 = Modbus::CalculateCRC16((uint8_t * ) pucSndBufferCur, usSndBufferCount );
        ucRTUBuf[usSndBufferCount++] = (uint8_t)(usCRC16 & 0xFF);
        ucRTUBuf[usSndBufferCount++] = (uint8_t)(usCRC16 >> 8);

        TXState = STATE_TX_XMIT;
        CommunicationsInterrupts(false, true);
    }
    else{
        eStatus = MB_EIO;
    }
    __enable_irq();
    return eStatus;
}
bool ModbusRTU::RTUReceiveFSM(){
    bool xTaskNeedSwitch = false;
    uint8_t ucByte;

    assert_param(TXState == STATE_TX_IDLE);
    (void)ReceiveByte((uint8_t* )&ucByte);
    switch (RXState){
        case STATE_RX_INIT:
            EnableTimer();
            break;
        case STATE_RX_ERROR:
            EnableTimer();
            break;
        case STATE_RX_IDLE:
            usRcvBufferPos = 0;
            ucRTUBuf[usRcvBufferPos++] = ucByte;
            RXState = STATE_RX_RCV;
            EnableTimer();
            break;
        case STATE_RX_RCV:
            if(usRcvBufferPos < MB_SER_PDU_SIZE_MAX){
                ucRTUBuf[usRcvBufferPos++] = ucByte;
            }
            else{
                RXState = STATE_RX_ERROR;
            }
            EnableTimer();
            break;
    }
    return xTaskNeedSwitch;
}
bool ModbusRTU::RTUTransmitFSM(){
    bool xNeedPoll = false;
    assert_param(RXState == STATE_RX_IDLE);
    switch (TXState){
        case STATE_TX_IDLE:
            CommunicationsInterrupts(true, false);
            break;
        case STATE_TX_XMIT:
            if(usSndBufferCount != 0 ){
                TransmitByte((int8_t)*pucSndBufferCur);
                pucSndBufferCur++;
                usSndBufferCount--;
            }
            else{
                xNeedPoll = EventPost(EV_FRAME_SENT);
                CommunicationsInterrupts(true, false);
                TXState = STATE_TX_IDLE;
            }
            break;
        default:
        	break;
    }
    return xNeedPoll;
}
bool ModbusRTU::RTUTimerExpired(){
    bool xNeedPoll = false;
    switch (RXState){
        case STATE_RX_INIT:
            xNeedPoll = EventPost(EV_READY);
            break;
        case STATE_RX_RCV:
            xNeedPoll = EventPost(EV_FRAME_RECEIVED);
            break;
        case STATE_RX_ERROR:
            break;
        default:
            assert_param((RXState == STATE_RX_INIT) || (RXState == STATE_RX_RCV) || (RXState == STATE_RX_ERROR));
    }
    DisableTimer();
    RXState = STATE_RX_IDLE;
    return xNeedPoll;
}
bool ModbusRTU::RTUMasterTimerExpired(void){
    bool xNeedPoll = false;
    switch (RXState){
    case STATE_RX_INIT:
        xNeedPoll = EventPost(EV_READY);
        break;
    case STATE_RX_RCV:
        xNeedPoll = EventPost(EV_FRAME_RECEIVED);
        break;
    case STATE_RX_ERROR:
        MasterSetErrorType(EV_ERROR_RECEIVE_DATA);
        xNeedPoll = EventPost(EV_PROCESS_ERROR);
        break;
    default:
        assert_param((RXState == STATE_RX_INIT) || (RXState == STATE_RX_RCV) || (RXState == STATE_RX_ERROR) || (RXState == STATE_RX_IDLE));
        break;
    }
    RXState = STATE_RX_IDLE;

    switch (TXState){
        case STATE_TX_XFWR:
            if (xFrameIsBroadcast == false) {
                MasterSetErrorType(EV_ERROR_RESPOND_TIMEOUT);
                xNeedPoll = EventPost(EV_PROCESS_ERROR);
            }
            break;
        default:
            assert_param((eSndState == STATE_M_TX_XFWR) || (eSndState == STATE_M_TX_IDLE));
            break;
    }
    TXState = STATE_TX_IDLE;

    DisableTimer();
    /* If timer mode is convert delay, the master event then turns EV_MASTER_EXECUTE status. */
    if (TimerMode == MB_TMODE_CONVERT_DELAY){
        xNeedPoll = EventPost(EV_EXECUTE);
    }

    return xNeedPoll;
}



bool ModbusRTU::MasterRTUTransmitFSM(){
    bool xNeedPoll = false;
    assert_param(RXState == STATE_M_RX_IDLE);

    switch (TXState){
        case STATE_TX_IDLE:
            CommunicationsInterrupts(true, false);
            break;
        case STATE_TX_XMIT:
            if(usSndBufferCount != 0){
                TransmitByte((int8_t)*pucSndBufferCur);
                pucSndBufferCur++;
                usSndBufferCount--;
            }
            else{
                xFrameIsBroadcast = (ucMasterRTUSndBuf[MB_SER_PDU_ADDR_OFF] == MB_ADDRESS_BROADCAST) ? true : false;
                CommunicationsInterrupts(true, false);
                TXState = STATE_TX_XFWR;
                if (xFrameIsBroadcast == true){
                    //vMBMasterPortTimersConvertDelayEnable( );
                }
                else{
                    //vMBMasterPortTimersRespondTimeoutEnable( );
                }
            }
            break;

        default:
            break;
        }
    return xNeedPoll;
}


void ModbusRTU::SetBits(uint8_t * ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits, uint8_t ucValue){
    uint16_t          usWordBuf;
    uint16_t          usMask;
    uint16_t          usByteOffset;
    uint16_t          usNPreBits;
    uint16_t          usValue = ucValue;

    assert_param(ucNBits <= 8);
    assert_param((size_t)8U == sizeof(uint8_t) * 8);
    usByteOffset = (uint16_t)( ( usBitOffset ) / 8U);
    usNPreBits = (uint16_t)( usBitOffset - usByteOffset * 8U);
    usValue <<= usNPreBits;
    usMask = (uint16_t)((1<<(uint16_t)ucNBits) - 1);
    usMask <<= usBitOffset - usByteOffset * 8U;
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << 8U;
    usWordBuf = (uint16_t)( (usWordBuf & (~usMask) ) | usValue);
    ucByteBuf[usByteOffset] = (uint8_t)(usWordBuf & 0xFF);
    ucByteBuf[usByteOffset + 1] = (uint8_t)(usWordBuf>>8U);
}
uint8_t ModbusRTU::GetBits(uint8_t * ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits){
    uint16_t          usWordBuf;
    uint16_t          usMask;
    uint16_t          usByteOffset;
    uint16_t          usNPreBits;

    usByteOffset = (uint16_t)((usBitOffset)/8U);
    usNPreBits = (uint16_t)(usBitOffset - usByteOffset * 8U);
    usMask = (uint16_t)((1<<(uint16_t)ucNBits) - 1);
    usWordBuf = ucByteBuf[usByteOffset];
    usWordBuf |= ucByteBuf[usByteOffset + 1] << 8U;
    usWordBuf >>= usNPreBits;
    usWordBuf &= usMask;
    return (uint8_t)usWordBuf;
}

bool ModbusRTU::IsMasterMode(void){
    return InMasterMode;
}
void ModbusRTU::RunInMasterMode(bool IsMasterMode){
    InMasterMode = IsMasterMode;
}

void ModbusRTU::MasterGetRTUTXBuffer(uint8_t ** pucFrame){
    *pucFrame = (uint8_t * ) ucMasterRTUSndBuf;
}
void ModbusRTU::MasterGetPDUTXBuffer(uint8_t ** pucFrame){
    *pucFrame = (uint8_t * ) &ucMasterRTUSndBuf[MB_SER_PDU_PDU_OFF];
}
void ModbusRTU::MasterSetPDUTXLength(uint16_t SendPDULength){
    usMasterSendPDULength = SendPDULength;
}
uint16_t ModbusRTU::MasterGetPDUTXLength(void){
    return usMasterSendPDULength;
}
void ModbusRTU::MasterSetTimeMode(MasterSetTimerMode eMBTimerMode){
    TimerMode = eMBTimerMode;
}
bool ModbusRTU::MasterRequestIsBroadcast(void){
    return xFrameIsBroadcast;
}
ModbusState ModbusRTU::MasterGetErrorType(void){
    return CurrentMasterStatus;
}
void ModbusRTU::MasterSetErrorType(ModbusState ErrorType){
    CurrentMasterStatus = ErrorType;
}
ModbusError ModbusRTU::MasterReqReadCoils(uint8_t ucSndAddr, uint16_t usCoilAddr, uint16_t usNCoils, int32_t lTimeOut){
    uint8_t *ucMBFrame;
    ModbusError eErrStatus = MB_ENOERR;

    if (ucSndAddr > MB_MASTER_TOTAL_SLAVE_NUM){eErrStatus = MB_EINVAL;}
    //else if (xMBMasterRunResTake(lTimeOut) == false){eErrStatus = MB_MRE_MASTER_BUSY;}
    else{
        MasterGetPDUTXBuffer(&ucMBFrame);
        MasterSetDestinationAddress(ucSndAddr);
        ucMBFrame[MB_PDU_FUNC_OFF]                 = MB_FUNC_READ_COILS;
        ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF]        = usCoilAddr >> 8;
        ucMBFrame[MB_PDU_REQ_READ_ADDR_OFF + 1]    = usCoilAddr;
        ucMBFrame[MB_PDU_FUNC_READ_COILCNT_OFF ]    = usNCoils >> 8;
        ucMBFrame[MB_PDU_FUNC_READ_COILCNT_OFF + 1] = usNCoils;
        MasterSetPDUTXLength(MB_PDU_SIZE_MIN + MB_PDU_REQ_READ_SIZE);
        (void)EventPost(EV_FRAME_SENT);
        //eErrStatus = eMBMasterWaitRequestFinish();
    }
    return eErrStatus;
}
