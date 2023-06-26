#include "ModbusRTU.h"

ModbusError ModbusRTU::MasterPoll(void){
    static uint8_t   *ucMBFrame;
    static uint8_t    ucRcvAddress;
    static uint8_t    ucFunctionCode;
    static uint16_t   usLength;
    static ModbusException eException;

    int             i , j;
    ModbusError    eStatus = MB_ENOERR;
    ModbusState    eEvent;
    ModbusState errorType;

    if((SystemState != STATE_ENABLED) && (SystemState != STATE_ESTABLISHED)){
        return MB_EILLSTATE;
    }
    if(EventRetrieve(&eEvent) == true){
        switch (eEvent){
            case EV_READY:
                SystemState = STATE_ESTABLISHED;
                break;
            case EV_FRAME_RECEIVED:
                eStatus = RTUReceive(&ucRcvAddress, &ucMBFrame, &usLength);
                /* Check if the frame is for us. If not ,send an error process event. */
                if ((eStatus == MB_ENOERR) && (ucRcvAddress == MasterGetDestinationAddress())){
                    (void)EventPost(EV_EXECUTE);
                }
                else{
                    MasterSetErrorType(EV_ERROR_RECEIVE_DATA);
                    (void)EventPost(EV_PROCESS_SUCCESS);
                }
                break;
            case EV_EXECUTE:
                ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
                eException = MB_EX_ILLEGAL_FUNCTION;
                if(ucFunctionCode >> 7) {
                    eException = (ModbusException)ucMBFrame[MB_PDU_DATA_OFF];
                }
                else{
                    for (i = 0; i < MB_FUNC_HANDLERS_MAX; i++){
                        if (Modbus::xFuncHandlers[i].ucFunctionCode == 0) {
                            break;
                        }
                        else if (Modbus::xFuncHandlers[i].ucFunctionCode == ucFunctionCode) {
                            RunInMasterMode(true);
                            if (MasterRequestIsBroadcast()) {
                                usLength = MasterGetPDUTXLength();
                                for(j = 1; j <= MB_MASTER_TOTAL_SLAVE_NUM; j++){
                                	MasterSetDestinationAddress(j);
                                    eException = Modbus::xFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
                                }
                            }
                            else {
                                eException = Modbus::xFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
                            }
                            RunInMasterMode(false);
                            break;
                        }
                    }
                }
                if (eException != MB_EX_NONE) {
                	MasterSetErrorType(EV_EXECUTE);
                    (void)EventPost(EV_PROCESS_SUCCESS);
                }
                else {
                    Modbus::RequestSuccessCB();
                }
                break;
            case EV_FRAME_SENT:
            	MasterGetPDUTXBuffer(&ucMBFrame);
                eStatus = RTUSend(MasterGetDestinationAddress(), ucMBFrame, MasterGetPDUTXLength());
                break;
            case EV_PROCESS_SUCCESS:
                errorType = MasterGetErrorType();
                MasterGetPDUTXBuffer(&ucMBFrame);
                switch (errorType) {
                    case EV_ERROR_RESPOND_TIMEOUT:
                        Modbus::ErrorRespondTimeoutCB(MasterGetDestinationAddress(), ucMBFrame, MasterGetPDUTXLength());
                        break;
                    case EV_ERROR_RECEIVE_DATA:
                        Modbus::ErrorRespondTimeoutCB(MasterGetDestinationAddress(), ucMBFrame, MasterGetPDUTXLength());
                        break;
                    case EV_ERROR_EXECUTE_FUNCTION:
                        Modbus::ErrorExecuteFunctionCB(MasterGetDestinationAddress(), ucMBFrame, MasterGetPDUTXLength());
                        break;
                    default:
                    	break;
                }
                break;
            default:
                break;
        }
    }
    return MB_ENOERR;
}




ModbusError ModbusRTU::Poll(void){
    static uint8_t   *ucMBFrame;
    static uint8_t    ucRcvAddress;
    static uint8_t    ucFunctionCode;
    static uint16_t   usLength;
    static ModbusException eException;

    int             i;
    ModbusError    eStatus = MB_ENOERR;
    ModbusState    eEvent;
    if(SystemState != STATE_ENABLED){
        return MB_EILLSTATE;
    }
    if(EventRetrieve(&eEvent) == true){
        switch (eEvent){
            case EV_READY:
                break;
            case EV_FRAME_RECEIVED:
                eStatus = RTUReceive(&ucRcvAddress, &ucMBFrame, &usLength);
                if(eStatus == MB_ENOERR){
                    if((ucRcvAddress == ucMBAddress)||(ucRcvAddress == MB_ADDRESS_BROADCAST)){
                        (void)EventPost(EV_EXECUTE);
                    }
                }
                break;
            case EV_EXECUTE:
                ucFunctionCode = ucMBFrame[MB_PDU_FUNC_OFF];
                eException = MB_EX_ILLEGAL_FUNCTION;
                for(i=0;i<MB_FUNC_HANDLERS_MAX;i++){
                    if(Modbus::xFuncHandlers[i].ucFunctionCode == 0){
                        break;
                    }
                    else if(Modbus::xFuncHandlers[i].ucFunctionCode == ucFunctionCode){
                        eException = Modbus::xFuncHandlers[i].pxHandler(ucMBFrame, &usLength);
                        break;
                    }
                }
                if(ucRcvAddress != MB_ADDRESS_BROADCAST){
                    if(eException != MB_EX_NONE){
                        usLength = 0;
                        ucMBFrame[usLength++] = (uint8_t)(ucFunctionCode | MB_FUNC_ERROR);
                        ucMBFrame[usLength++] = eException;
                    }
                    //if((eMBCurrentMode == MB_ASCII) && MB_ASCII_TIMEOUT_WAIT_BEFORE_SEND_MS){
                    //    vMBPortTimersDelay(MB_ASCII_TIMEOUT_WAIT_BEFORE_SEND_MS);
                    //}                
                    eStatus = RTUSend(ucMBAddress, ucMBFrame, usLength);
                }
                break;
            case EV_FRAME_SENT:
                break;
            default:
                break;
        }
    }
    return MB_ENOERR;
}
