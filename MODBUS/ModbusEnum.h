enum ModbusException{
    MBUS_EXCEPT_NONE                            = 0x00,
    MBUS_EXCEPT_ILLEGAL_FUNCTION                = 0x01,
    MBUS_EXCEPT_ILLEGAL_DATA_ADDRESS            = 0x02,
    MBUS_EXCEPT_ILLEGAL_DATA_VALUE              = 0x03,
    MBUS_EXCEPT_SLAVE_DEVICE_FAILURE            = 0x04,
    MBUS_EXCEPT_ACKNOWLEDGE                     = 0x05,
    MBUS_EXCEPT_SLAVE_BUSY                      = 0x06,
    MBUS_EXCEPT_MEMORY_PARITY_ERROR             = 0x08,
    MBUS_EXCEPT_GATEWAY_PATH_FAILED             = 0x0A,
    MBUS_EXCEPT_GATEWAY_TGT_FAILED              = 0x0B
};
enum ModbusFunction{
    MBUS_FUNC_None                              = 0,
    MBUS_FUNC_ReadDiscreteInputs	            = 2,	
    MBUS_FUNC_ReadCoils	                        = 1,
    MBUS_FUNC_WriteSingleCoil	                = 5,
    MBUS_FUNC_WriteMultipleCoils	            = 15,	
    MBUS_FUNC_ReadInputRegisters	            = 4,	
    MBUS_FUNC_ReadHoldingRegisters	            = 3,	
    MBUS_FUNC_WriteSingleHoldingRegister	    = 6,	
    MBUS_FUNC_WriteMultipleHoldingRegisters	    = 16,	
    MBUS_FUNC_RWMultipleRegisters	            = 23,	
    MBUS_FUNC_MaskWriteRegister	                = 22,	
    MBUS_FUNC_ReadFIFOQueue	                    = 24	
};

enum ModbusState{
    MBUS_SYS_IDLE                                   = 0x001,
    MBUS_SYS_RECEIVED_BYTE                          = 0X002,
    MBUS_SYS_RECEIVED_FRAME                         = 0x004,
    MBUS_SYS_TRANSMIT                               = 0x008,
    
    MBUS_SYS_FRAME_SENT                             = 0x010,
    MBUS_SYS_ESTABLISHED                            = 0x020,
    MBUS_SYS_PROCESS_SUCCESS                        = 0x040,
    MBUS_SYS_PROCESS_ERROR                          = 0x080,
    MBUS_SYS_ERROR_RESPOND_TIMEOUT                  = 0x100,  
    MBUS_SYS_ERROR_RECEIVE_DATA                     = 0x200,  
    MBUS_SYS_ERROR_EXECUTE_FUNCTION                 = 0x400
};

enum ModbusRXState{
    MBUS_RX_INIT                                    = 0x01,              
    MBUS_RX_IDLE                                    = 0x02,              
    MBUS_RX_RCV                                     = 0x04,               
    MBUS_RX_ERROR                                   = 0x08              
};

enum ModbusTXState{
    MBUS_TX_IDLE                                    = 0x01,   
    MBUS_TX_READY                                   = 0x02,  
    MBUS_TX_PROCESS                                 = 0x04,
    MBUS_TX_EMITTED                                 = 0x08
};

enum ModbusError{
    MB_ENOERR                   = 0x001,
    MB_ENOREG                   = 0x002,
    MB_EINVAL                   = 0x004,
    MB_EPORTERR                 = 0x008,
    MB_ENORES                   = 0x010,
    MB_EIO                      = 0x020,
    MB_EILLSTATE                = 0x040,
    MB_ETIMEDOUT                = 0x080 
};