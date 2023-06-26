


#define REG_INPUT_START                         1000
#define REG_INPUT_NREGS                         8

#define MB_SER_PDU_SIZE_MIN                     4   
#define MB_SER_PDU_SIZE_MAX                     256 
#define MB_SER_PDU_SIZE_CRC                     2   
#define MB_SER_PDU_ADDR_OFF                     0   
#define MB_SER_PDU_PDU_OFF                      1   

#define MB_FUNC_OTHER_REP_SLAVEID_BUF           32

#define MB_PDU_SIZE_MAX                         253
#define MB_PDU_SIZE_MIN                         1  
#define MB_PDU_FUNC_OFF                         0  
#define MB_PDU_DATA_OFF                         1  

#define MB_PDU_FUNC_READ_ADDR_OFF               MB_PDU_DATA_OFF
#define MB_PDU_FUNC_READ_COILCNT_OFF            MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_READ_SIZE                   4
#define MB_PDU_FUNC_READ_COILCNT_MAX            0x07D0

#define MB_PDU_FUNC_WRITE_ADDR_OFF              MB_PDU_DATA_OFF
#define MB_PDU_FUNC_WRITE_VALUE_OFF             MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_WRITE_SIZE                  4

#define MB_PDU_FUNC_WRITE_MUL_REGCNT_OFF        MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_WRITE_MUL_REGCNT_MAX        0x0078

#define MB_PDU_FUNC_WRITE_MUL_ADDR_OFF          MB_PDU_DATA_OFF
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_OFF       MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_WRITE_MUL_BYTECNT_OFF       MB_PDU_DATA_OFF + 4
#define MB_PDU_FUNC_WRITE_MUL_VALUES_OFF        MB_PDU_DATA_OFF + 5
#define MB_PDU_FUNC_WRITE_MUL_SIZE_MIN          5
#define MB_PDU_FUNC_WRITE_MUL_COILCNT_MAX       0x07B0

#define MB_PDU_FUNC_READ_ADDR_OFF               MB_PDU_DATA_OFF
#define MB_PDU_FUNC_READ_REGCNT_OFF             MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_READ_SIZE                   4
#define MB_PDU_FUNC_READ_REGCNT_MAX             0x007D

#define MB_PDU_FUNC_READ_RSP_BYTECNT_OFF        MB_PDU_DATA_OFF 

#define MB_PDU_FUNC_READ_DISCCNT_OFF            MB_PDU_DATA_OFF + 2 
#define MB_PDU_FUNC_READ_DISCCNT_MAX            0x07D0 

#define MB_PDU_FUNC_READWRITE_READ_ADDR_OFF     MB_PDU_DATA_OFF + 0
#define MB_PDU_FUNC_READWRITE_READ_REGCNT_OFF   MB_PDU_DATA_OFF + 2
#define MB_PDU_FUNC_READWRITE_WRITE_ADDR_OFF    MB_PDU_DATA_OFF + 4
#define MB_PDU_FUNC_READWRITE_WRITE_REGCNT_OFF  MB_PDU_DATA_OFF + 6
#define MB_PDU_FUNC_READWRITE_BYTECNT_OFF       MB_PDU_DATA_OFF + 8
#define MB_PDU_FUNC_READWRITE_WRITE_VALUES_OFF  MB_PDU_DATA_OFF + 9
#define MB_PDU_FUNC_READWRITE_SIZE_MIN          9

#define MB_PDU_REQ_READ_ADDR_OFF                MB_PDU_DATA_OFF + 0
#define MB_PDU_REQ_READ_REGCNT_OFF              MB_PDU_DATA_OFF + 2
#define MB_PDU_REQ_READ_SIZE                    4
#define MB_PDU_FUNC_READ_BYTECNT_OFF            MB_PDU_DATA_OFF + 0
#define MB_PDU_FUNC_READ_VALUES_OFF             MB_PDU_DATA_OFF + 1
#define MB_PDU_FUNC_READ_SIZE_MIN               1

#define MB_PDU_FUNC_READ_RSP_BYTECNT_OFF        MB_PDU_DATA_OFF

#define MB_MASTER_DELAY_MS_CONVERT              200
#define MB_MASTER_TIMEOUT_MS_RESPOND            100
#define MB_MASTER_TOTAL_SLAVE_NUM               16