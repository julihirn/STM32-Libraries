#ifndef _MODBUS_RTU_H
    #define _MODBUS_RTU_H
    enum ModBusSystemState{
        MBUS_INITALISE = 0x00,
		MBUS_IDLE = 0x01,
		MBUS_TRANSMISSION = 0x02,
		MBUS_RECIEVE = 0x03,
		MBUS_WAITING = 0x04
    };
    class MODBUS_RTU{
        public:
            MODBUS_RTU();
        private:
            //Handles
			static UART_HandleTypeDef *Handle;
			static GPIO_TypeDef * TXEnablePort;
			static int16_t TXEnablePin;
    };
#endif