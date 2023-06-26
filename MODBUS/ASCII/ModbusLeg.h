#if 1 == 0

#ifndef _MODBUSL_H
    #define _MODBUSL_H
    #include "Driver.h"
    #include "Generics.h"

    #include "ModbusFlags.h"

    enum ModbusState{
        MBUS_INITALISE = 0x00,
        MBUS_IDLE = 0x01,
        MBUS_TRANSMITTING = 0x02,
        MBUS_RECIEVING = 0x03
    };
    enum ModbusCommand{
        MBUS_ReadDiscreteInputs = 0x02,
        MBUS_ReadCoils = 0x01,
        MBUS_WriteSingleCoil = 0x05,
        MBUS_WriteMultipleCoils = 0x0F,
        MBUS_ReadInputRegisters = 0x04,
        MBUS_ReadMultipleHoldingRegisters = 0x03,
        MBUS_WriteSingleHoldingRegister = 0x06,
        MBUS_WriteMultipleHoldingRegisters = 0x10
    };

#endif
#endif