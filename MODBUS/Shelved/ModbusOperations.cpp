#include "Modbus.h"

ModbusError Modbus::RegisterCB(uint8_t ucFunctionCode, Modbus::pxMBFunctionHandler pxHandler ){
    int             i;
    ModbusError    eStatus;

    if((0 < ucFunctionCode) && (ucFunctionCode <= 127)){
        __enable_irq();
        if(pxHandler != NULL){
            for(i = 0; i < MB_FUNC_HANDLERS_MAX; i++){
                if((Modbus::xFuncHandlers[i].pxHandler == NULL) || (Modbus::xFuncHandlers[i].pxHandler == pxHandler)){
                    Modbus::xFuncHandlers[i].ucFunctionCode = ucFunctionCode;
                    Modbus::xFuncHandlers[i].pxHandler = pxHandler;
                    break;
                }
            }
            eStatus = (i != MB_FUNC_HANDLERS_MAX) ? MB_ENOERR : MB_ENORES;
        }
        else{
            for(i = 0; i < MB_FUNC_HANDLERS_MAX; i++){
                if(Modbus::xFuncHandlers[i].ucFunctionCode == ucFunctionCode){
                    Modbus::xFuncHandlers[i].ucFunctionCode = 0;
                    Modbus::xFuncHandlers[i].pxHandler = NULL;
                    break;
                }
            }
            eStatus = MB_ENOERR;
        }
        __disable_irq();
    }
    else{
        eStatus = MB_EINVAL;
    }
    return eStatus;
}
ModbusError Modbus::RegInputCB(uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs){
    ModbusError    eStatus = MB_ENOERR;
    int             iRegIndex;
    if((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)){
        iRegIndex = (int)(usAddress - usRegInputStart);
        while(usNRegs > 0){
            *pucRegBuffer++ = (uint8_t)(usRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (uint8_t)(usRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    }
    else{
        eStatus = MB_ENOREG;			
    }
    return eStatus;
}
ModbusError Modbus::RegHoldingCB(uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, ModbusOperation eMode){
    return MB_ENOREG;
}
ModbusError Modbus::RegCoilsCB(uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNCoils, ModbusOperation eMode){
    return MB_ENOREG;
}
ModbusError Modbus::RegDiscreteCB(uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNDiscrete){
    return MB_ENOREG;
}

ModbusError Modbus::ErrorRespondTimeoutCB(uint8_t ucDestAddress, const uint8_t* pucPDUData, uint16_t ucPDULength) {
	 return MB_ENOREG;
}
ModbusError Modbus::ErrorReceiveDataCB(uint8_t ucDestAddress, const uint8_t* pucPDUData, uint16_t ucPDULength) {
	 return MB_ENOREG;
}
ModbusError Modbus::ErrorExecuteFunctionCB(uint8_t ucDestAddress, const uint8_t* pucPDUData, uint16_t ucPDULength) {
	 return MB_ENOREG;
}
ModbusError Modbus::RequestSuccessCB(void) {
	 return MB_ENOREG;
}


ModbusException Modbus::ReadCoils(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usCoilCount;
    uint8_t           ucNBytes;
    uint8_t          *FrameCur;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress =  (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;
        usCoilCount = (uint16_t)(Frame[MB_PDU_FUNC_READ_COILCNT_OFF] << 8);
        usCoilCount |= (uint16_t)(Frame[MB_PDU_FUNC_READ_COILCNT_OFF + 1]);
        if((usCoilCount >= 1 ) && (usCoilCount < MB_PDU_FUNC_READ_COILCNT_MAX )){
            FrameCur = &Frame[MB_PDU_FUNC_OFF];
            *Length = MB_PDU_FUNC_OFF;
            *FrameCur++ = MB_FUNC_READ_COILS;
            *Length += 1;
            if((usCoilCount & 0x0007)!= 0){
                ucNBytes = (uint8_t)(usCoilCount / 8 + 1);
            }
            else{
                ucNBytes = (uint8_t)(usCoilCount / 8);
            }
            *FrameCur++ = ucNBytes;
            *Length += 1;
            eRegStatus = RegCoilsCB(FrameCur, usRegAddress, usCoilCount, MB_REG_READ);
            /* If an error occured convert it into a Modbus exception. */
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length += ucNBytes;;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::WriteCoil(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint8_t           ucBuf[2];
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length == (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1]);
        usRegAddress++;
        if((Frame[MB_PDU_FUNC_WRITE_VALUE_OFF + 1] == 0x00) && ((Frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF) || (Frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0x00))){
            ucBuf[1] = 0;
            if(Frame[MB_PDU_FUNC_WRITE_VALUE_OFF] == 0xFF){
                ucBuf[0] = 1;
            }
            else{
                ucBuf[0] = 0;
            }
            eRegStatus = RegCoilsCB( &ucBuf[0], usRegAddress, 1, MB_REG_WRITE);
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::WriteMultipleCoils(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usCoilCnt;
    uint8_t           ucByteCount;
    uint8_t           ucByteCountVerify;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length > (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1]);
        usRegAddress++;
        usCoilCnt = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF] << 8);
        usCoilCnt |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF + 1]);
        ucByteCount = Frame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];
        if((usCoilCnt & 0x0007) != 0){
            ucByteCountVerify = (uint8_t)(usCoilCnt / 8 + 1);
        }
        else{
            ucByteCountVerify = (uint8_t)(usCoilCnt / 8);
        }
        if((usCoilCnt >= 1) && (usCoilCnt <= MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX ) && (ucByteCountVerify == ucByteCount)){
            eRegStatus = RegCoilsCB(&Frame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF], usRegAddress, usCoilCnt, MB_REG_WRITE);
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::ReadDiscreteInputs(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usDiscreteCnt;
    uint8_t           ucNBytes;
    uint8_t          *FrameCur;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;
        usDiscreteCnt = (uint16_t)(Frame[MB_PDU_FUNC_READ_DISCCNT_OFF] << 8);
        usDiscreteCnt |= (uint16_t)(Frame[MB_PDU_FUNC_READ_DISCCNT_OFF + 1]);
        if((usDiscreteCnt >= 1) && (usDiscreteCnt < MB_PDU_FUNC_READ_DISCCNT_MAX)){
            /* Set the current PDU data pointer to the beginning. */
            FrameCur = &Frame[MB_PDU_FUNC_OFF];
            *Length = MB_PDU_FUNC_OFF;
            *FrameCur++ = MB_FUNC_READ_DISCRETE_INPUTS;
            *Length += 1;
            if((usDiscreteCnt & 0x0007) != 0){
                ucNBytes = (uint8_t)(usDiscreteCnt / 8 + 1);
            }
            else{
                ucNBytes = (uint8_t)(usDiscreteCnt / 8);
            }
            *FrameCur++ = ucNBytes;
            *Length += 1;
            eRegStatus = RegDiscreteCB(FrameCur, usRegAddress, usDiscreteCnt);
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length += ucNBytes;;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::WriteHoldingRegister(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length == (MB_PDU_FUNC_WRITE_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_ADDR_OFF + 1]);
        usRegAddress++;
        eRegStatus = RegHoldingCB(&Frame[MB_PDU_FUNC_WRITE_VALUE_OFF], usRegAddress, 1, MB_REG_WRITE);
        if(eRegStatus != MB_ENOERR){
            eStatus = ErrorException(eRegStatus);
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::WriteMultipleHoldingRegister(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usRegCount;
    uint8_t           ucRegByteCount;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if( *Length >= (MB_PDU_FUNC_WRITE_MUL_SIZE_MIN + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_ADDR_OFF + 1]);
        usRegAddress++;
        usRegCount = (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF] << 8);
        usRegCount |= (uint16_t)(Frame[MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF + 1]);
        ucRegByteCount = Frame[MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF];
        if((usRegCount >= 1) && (usRegCount <= MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX) && (ucRegByteCount == (uint8_t)(2 * usRegCount))){
            eRegStatus = RegHoldingCB( &Frame[MB_PDU_FUNC_WRITE_MUL_VALUES_OFF], usRegAddress, usRegCount, MB_REG_WRITE);
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length = MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::ReadHoldingRegister(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usRegCount;
    uint8_t          *FrameCur;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;
        usRegCount = (uint16_t)(Frame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8);
        usRegCount = (uint16_t)(Frame[MB_PDU_FUNC_READ_REGCNT_OFF + 1]);
        if((usRegCount >= 1) && (usRegCount <= MB_PDU_FUNC_READ_REGCNT_MAX)){
            FrameCur = &Frame[MB_PDU_FUNC_OFF];
            *Length = MB_PDU_FUNC_OFF;
            *FrameCur++ = MB_FUNC_READ_HOLDING_REGISTER;
            *Length += 1;
            *FrameCur++ = (uint8_t)(usRegCount * 2);
            *Length += 1;
            eRegStatus = RegHoldingCB(FrameCur, usRegAddress, usRegCount, MB_REG_READ);
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length += usRegCount * 2;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusException Modbus::ReadWriteMultipleHoldingRegister(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegReadAddress;
    uint16_t          usRegReadCount;
    uint16_t          usRegWriteAddress;
    uint16_t          usRegWriteCount;
    uint8_t           ucRegWriteByteCount;
    uint8_t          *FrameCur;
    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;
    if(*Length >= (MB_PDU_FUNC_READWRITE_SIZE_MIN + MB_PDU_SIZE_MIN)){
        usRegReadAddress = (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF] << 8U);
        usRegReadAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_READ_ADDR_OFF + 1]);
        usRegReadAddress++;
        usRegReadCount = (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF] << 8U);
        usRegReadCount |= (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF + 1]);
        usRegWriteAddress = (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF] << 8U);
        usRegWriteAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF + 1]);
        usRegWriteAddress++;
        usRegWriteCount = (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF] << 8U);
        usRegWriteCount |= (uint16_t)(Frame[MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF + 1]);
        ucRegWriteByteCount = Frame[MB_PDU_FUNC_READWRITE_BYTECNT_OFF];
        if((usRegReadCount >= 1) && (usRegReadCount <= 0x7D) && (usRegWriteCount >= 1) && (usRegWriteCount <= 0x79) && ((2 * usRegWriteCount) == ucRegWriteByteCount)){
            eRegStatus = RegHoldingCB(&Frame[MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF], usRegWriteAddress, usRegWriteCount, MB_REG_WRITE);
            if(eRegStatus == MB_ENOERR){
                FrameCur = &Frame[MB_PDU_FUNC_OFF];
                *Length = MB_PDU_FUNC_OFF;
                *FrameCur++ = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
                *Length += 1;
                *FrameCur++ = (uint8_t) (usRegReadCount * 2);
                *Length += 1;
                eRegStatus = RegHoldingCB(FrameCur, usRegReadAddress, usRegReadCount, MB_REG_READ);
                if(eRegStatus == MB_ENOERR){
                    *Length += 2 * usRegReadCount;
                }
            }
            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    return eStatus;
}
ModbusException Modbus::ReadInputRegister(uint8_t * Frame, uint16_t * Length){
    uint16_t          usRegAddress;
    uint16_t          usRegCount;
    uint8_t          *FrameCur;

    ModbusException    eStatus = MB_EX_NONE;
    ModbusError    eRegStatus;

    if(*Length == (MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN)){
        usRegAddress = (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF] << 8);
        usRegAddress |= (uint16_t)(Frame[MB_PDU_FUNC_READ_ADDR_OFF + 1]);
        usRegAddress++;

        usRegCount = (uint16_t)(Frame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8);
        usRegCount |= (uint16_t)(Frame[MB_PDU_FUNC_READ_REGCNT_OFF + 1]);

        if((usRegCount >= 1) && (usRegCount < MB_PDU_FUNC_READ_REGCNT_MAX)){
            FrameCur = &Frame[MB_PDU_FUNC_OFF];
            *Length = MB_PDU_FUNC_OFF;

            *FrameCur++ = MB_FUNC_READ_INPUT_REGISTER;
            *Length += 1;

            *FrameCur++ = (uint8_t)(usRegCount * 2);
            *Length += 1;

            eRegStatus = RegInputCB(FrameCur, usRegAddress, usRegCount);

            if(eRegStatus != MB_ENOERR){
                eStatus = ErrorException(eRegStatus);
            }
            else{
                *Length += usRegCount * 2;
            }
        }
        else{
            eStatus = MB_EX_ILLEGAL_DATA_VALUE;
        }
    }
    else{
        eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}
ModbusError Modbus::SetSlaveID(uint8_t ucSlaveID, bool xIsRunning, uint8_t const *pucAdditional, uint16_t usAdditionalLen){
    ModbusError    eStatus = MB_ENOERR;
    if(usAdditionalLen + 2 < MB_FUNC_OTHER_REP_SLAVEID_BUF){
        usMBSlaveIDLen = 0;
        ucMBSlaveID[usMBSlaveIDLen++] = ucSlaveID;
        ucMBSlaveID[usMBSlaveIDLen++] = (uint8_t)(xIsRunning ? 0xFF : 0x00);
        if(usAdditionalLen > 0){
            memcpy( &ucMBSlaveID[usMBSlaveIDLen], pucAdditional, (size_t)usAdditionalLen);
            usMBSlaveIDLen += usAdditionalLen;
        }
    }
    else{
        eStatus = MB_ENORES;
    }
    return eStatus;
}
ModbusException Modbus::ReportSlaveID(uint8_t * Frame, uint16_t * Length){
    memcpy(&Frame[MB_PDU_DATA_OFF], &ucMBSlaveID[0], ( size_t)usMBSlaveIDLen);
    *Frame = (uint16_t)(MB_PDU_DATA_OFF + usMBSlaveIDLen);
    return MB_EX_NONE;
}
ModbusException Modbus::ErrorException(ModbusError ErrorCode){
    ModbusException    eStatus;
    switch (ErrorCode){
        case MB_ENOERR:
            eStatus = MB_EX_NONE;
            break;
        case MB_ENOREG:
            eStatus = MB_EX_ILLEGAL_DATA_ADDRESS;
            break;
        case MB_ETIMEDOUT:
            eStatus = MB_EX_SLAVE_BUSY;
            break;
        default:
            eStatus = MB_EX_SLAVE_DEVICE_FAILURE;
            break;
    }
    return eStatus;
}
