#include "modbus_rtu.h"

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

void Transmit(){
    
}