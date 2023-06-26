#ifndef __MODBUS_H
    #define __MODBUS_H

    #include "Driver.h"
    #include "Generics.h"

    #ifdef __cplusplus
        extern "C" {
    #endif
        #include "ModbusEnums.h"
        #include "ModbusFlags.h"
        #include "ModbusSettings.h"
    #ifdef __cplusplus
        }
    #endif
    namespace Modbus{
        typedef   ModbusException( *pxMBFunctionHandler) (uint8_t * Frame, uint16_t * Length);
        struct FunctionHandler{
            uint8_t ucFunctionCode;
            pxMBFunctionHandler pxHandler;
        };
        uint16_t        CalculateCRC16(uint8_t * Frame, uint16_t Length);

        ModbusError     RegisterCB(uint8_t, pxMBFunctionHandler);
        ModbusError     RegInputCB(uint8_t *, uint16_t, uint16_t);
        ModbusError     RegHoldingCB(uint8_t *, uint16_t, uint16_t, ModbusOperation);
        ModbusError     RegCoilsCB(uint8_t *, uint16_t, uint16_t, ModbusOperation);
        ModbusError     RegDiscreteCB(uint8_t *, uint16_t, uint16_t);

        ModbusError     ErrorRespondTimeoutCB(uint8_t, const uint8_t*, uint16_t);
        ModbusError     ErrorReceiveDataCB(uint8_t, const uint8_t*, uint16_t);
        ModbusError     ErrorExecuteFunctionCB(uint8_t, const uint8_t*, uint16_t);
        ModbusError     RequestSuccessCB(void);

        ModbusException ReadCoils(uint8_t *, uint16_t *);
        ModbusException WriteCoil(uint8_t *, uint16_t *);
        ModbusException WriteMultipleCoils(uint8_t *, uint16_t *);
        ModbusException ReadDiscreteInputs(uint8_t *, uint16_t *);
        ModbusException WriteHoldingRegister(uint8_t *, uint16_t *);
        ModbusException WriteMultipleHoldingRegister(uint8_t *, uint16_t *);
        ModbusException ReadHoldingRegister(uint8_t *, uint16_t *);
        ModbusException ReadWriteMultipleHoldingRegister(uint8_t *, uint16_t *);
        ModbusException ReadInputRegister(uint8_t *, uint16_t *);
        ModbusError     SetSlaveID(uint8_t, bool, uint8_t const *, uint16_t);
        ModbusException ReportSlaveID(uint8_t *, uint16_t *);

        ModbusException ErrorException(ModbusError);

        static FunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
            {MB_FUNC_OTHER_REPORT_SLAVEID,          ReportSlaveID},
            {MB_FUNC_READ_INPUT_REGISTER,           ReadInputRegister},
            {MB_FUNC_READ_HOLDING_REGISTER,         ReadHoldingRegister},
            {MB_FUNC_WRITE_MULTIPLE_REGISTERS,      WriteMultipleHoldingRegister},
            {MB_FUNC_WRITE_REGISTER,                WriteHoldingRegister},
            {MB_FUNC_READWRITE_MULTIPLE_REGISTERS,  ReadWriteMultipleHoldingRegister},
            {MB_FUNC_READ_COILS,                    ReadCoils},
            {MB_FUNC_WRITE_SINGLE_COIL,             WriteCoil},
            {MB_FUNC_WRITE_MULTIPLE_COILS,          WriteMultipleCoils},
            {MB_FUNC_READ_DISCRETE_INPUTS,          ReadDiscreteInputs},
        };
        //static xMBFunctionHandler xFuncHandlers[MB_FUNC_HANDLERS_MAX] = {
        //    {MB_FUNC_OTHER_REPORT_SLAVEID,          eMBFuncReportSlaveID},
        //    {MB_FUNC_READ_INPUT_REGISTER,           eMBMasterFuncReadInputRegister},
        //    {MB_FUNC_READ_HOLDING_REGISTER,         eMBMasterFuncReadHoldingRegister},
        //    {MB_FUNC_WRITE_MULTIPLE_REGISTERS,      eMBMasterFuncWriteMultipleHoldingRegister},
        //    {MB_FUNC_WRITE_REGISTER,                eMBMasterFuncWriteHoldingRegister},
        //    {MB_FUNC_READWRITE_MULTIPLE_REGISTERS,  eMBMasterFuncReadWriteMultipleHoldingRegister},
        //    {MB_FUNC_READ_COILS,                    eMBMasterFuncReadCoils},
        //    {MB_FUNC_WRITE_SINGLE_COIL,             eMBMasterFuncWriteCoil},
        //    {MB_FUNC_WRITE_MULTIPLE_COILS,          eMBMasterFuncWriteMultipleCoils},
        //    {MB_FUNC_READ_DISCRETE_INPUTS,          eMBMasterFuncReadDiscreteInputs},
        //};

        uint16_t ucMBSlaveID[MB_FUNC_OTHER_REP_SLAVEID_BUF];
        uint16_t usMBSlaveIDLen;

        uint16_t usRegInputStart = REG_INPUT_START;
        uint16_t usRegInputBuf[REG_INPUT_NREGS];
    };
#endif
