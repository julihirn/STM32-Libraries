enum ModbusState{
    EV_READY                    = 0x001,
    EV_FRAME_RECEIVED           = 0x002,
    EV_EXECUTE                  = 0x004,
    EV_FRAME_SENT               = 0x008,
    EV_ESTABLISHED              = 0x010,
    EV_PROCESS_SUCCESS          = 0x020,
    EV_PROCESS_ERROR            = 0x040,
    EV_ERROR_RESPOND_TIMEOUT    = 0x080,  
    EV_ERROR_RECEIVE_DATA       = 0x100,  
    EV_ERROR_EXECUTE_FUNCTION   = 0x200
};
enum ModbusParity{
    MB_PAR_NONE                 = 0x00,
    MB_PAR_ODD                  = 0x01, 
    MB_PAR_EVEN                 = 0x02  
};

enum ModbusException{
    MB_EX_NONE                  = 0x00,
    MB_EX_ILLEGAL_FUNCTION      = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS  = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE    = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE  = 0x04,
    MB_EX_ACKNOWLEDGE           = 0x05,
    MB_EX_SLAVE_BUSY            = 0x06,
    MB_EX_MEMORY_PARITY_ERROR   = 0x08,
    MB_EX_GATEWAY_PATH_FAILED   = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED    = 0x0B
};

//enum MasterErrorEvent{
//    EV_ERROR_RESPOND_TIMEOUT    = 0x01,
//    EV_ERROR_RECEIVE_DATA       = 0x02,
//    EV_ERROR_EXECUTE_FUNCTION   = 0x04,
//};

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
enum ModbusOperation{
    MB_REG_READ                 = 0x00,
    MB_REG_WRITE                = 0x01 
};
enum ModbusSystemState{
    STATE_ENABLED               = 0x01,
    STATE_DISABLED              = 0x02,
    STATE_NOT_INITIALIZED       = 0x04,
    STATE_ESTABLISHED           = 0x08
};

enum ModbusRXState{
    STATE_RX_INIT               = 0x01,              
    STATE_RX_IDLE               = 0x02,              
    STATE_RX_RCV                = 0x04,               
    STATE_RX_ERROR              = 0x08              
};

enum ModbusTXState{
    STATE_TX_IDLE               = 0x01,              
    STATE_TX_XMIT               = 0x02,
    STATE_TX_XFWR               = 0x04
};
enum MasterSetTimerMode{
    MB_TMODE_T35,            
    MB_TMODE_RESPOND_TIMEOUT,
    MB_TMODE_CONVERT_DELAY   
};

