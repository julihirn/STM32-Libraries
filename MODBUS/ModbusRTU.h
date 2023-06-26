#ifndef _MODBUS_RTU_H
    #define _MODBUS_RTU_H

    #include "Driver.h"
    #include "Generics.h"

    #include "ModbusCRC.h"

    #ifdef __cplusplus
        extern "C" {
    #endif
        //#include "ModbusFlags.h"
        #include "ModbusEnum.h"
        #include "ModbusSettings.h"
    #ifdef __cplusplus
        }
    #endif

    //enum ModBusSystemState{
    //    MBUS_INITALISE = 0x00,
	//	MBUS_IDLE = 0x01,
	//	MBUS_TRANSMISSION = 0x02,
	//	MBUS_RECIEVE = 0x03,
	//	MBUS_WAITING = 0x04
    //};
    struct ModbusCoil{
        uint16_t Count;
        bool * Coils;
    };
    struct ModbusRegister{
        uint16_t Count;
        uint16_t * Registers;
    };
    struct ModbusExceptionEvent{
        ModbusFunction RaisedAt;
        ModbusException Exception;

    };
    class ModbusRTU{
        public:
            ModbusRTU(UART_HandleTypeDef *, uint8_t *, TIM_HandleTypeDef *, GPIO_TypeDef *, int16_t);
            ModbusRTU(UART_HandleTypeDef *, uint8_t *, TIM_HandleTypeDef *, GPIO_TypeDef *, int16_t, uint8_t);
            
            void Poll();

            //Attachable Read Event Handlers
            void AttachOnReadCoilsReturn(void (*OnReadCoilsReturn)(ModbusRTU *, ModbusCoil *));
            void AttachOnReadRegisterReturn(void (*OnReadRegisterReturn)(ModbusRTU *, ModbusRegister *));
            
            //Attachable Write Event Handlers
            void AttachOnWriteCoilReturn(void (*OnWriteCoilReturn)(ModbusRTU *, ModbusCoil *));
            void AttachOnWriteRegisterReturn(void (*OnWriteRegisterReturn)(ModbusRTU *, ModbusRegister *));
            
            //Attachable Exception Event Handlers
            void AttachOnExceptionRaised(void (*OnExceptionRaised)(ModbusRTU *, ModbusExceptionEvent *));
            
            //Data Reception Linking
            void AppendByte(uint8_t);
            volatile uint32_t * LinkSilenceTick();

            //Master Commands
            //Boolean Operations
            void ReadCoils(uint8_t Address, uint16_t Start, uint16_t Count);
            void ReadDiscreteInputs(uint8_t Address, uint16_t Start, uint16_t Count);
            void WriteSingleCoil(uint8_t Address, uint16_t Start, bool Value);

            //Integer Operations
            void ReadHoldingRegisters(uint8_t Address, uint16_t Start, uint16_t Count);
            void ReadInputRegisters(uint8_t Address, uint16_t Start, uint16_t Count);
            void WriteSingleRegister(uint8_t Address, uint16_t Start, uint16_t Value);
            void WriteMultipleRegisters(uint8_t Address, uint16_t Start, uint16_t Count, uint16_t * Data);

            bool IsAddressEqual(uint8_t Address, uint16_t Start);

            static void CleanUp(ModbusCoil * Input);
            static void CleanUp(ModbusRegister * Input);

            #if MODBUS_USE_WATCH_DOG ==1
                void LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog);
            #endif

        private:
            uint32_t                 LastReceivedTick;

            uint8_t                  LastDeviceAddress;
            uint16_t                 LastRegister;
            //Handles
			UART_HandleTypeDef      *HndlComms;
			GPIO_TypeDef            *HndlPortEnable;
			int16_t                  PinEnable;

            #if MODBUS_USE_WATCH_DOG ==1
                IWDG_HandleTypeDef   *hnl_watchdog = NULL;
            #endif

            int16_t                  AttachedEventHandlers;

            uint8_t                 *CommBuff;

            volatile uint32_t        TickSilence = 0;
            volatile uint32_t        TickLastSent = 0;

            TIM_HandleTypeDef       *HndlTmr35;

            bool                    IsMaster = false;
            uint8_t                 SlaveAddress = 2;

            volatile ModbusState    SystemState         = MBUS_SYS_IDLE;
            volatile ModbusState    PreviousSystemState = MBUS_SYS_IDLE;
            volatile ModbusState    LatchedSystemState  = MBUS_SYS_IDLE;

            volatile ModbusRXState  RXState = MBUS_RX_IDLE;
            volatile ModbusTXState  TXState = MBUS_TX_IDLE;
            
            volatile uint8_t        RXBuffer[MODBUS_RTU_RXBUFFER_MAX];
            volatile uint8_t        TXBuffer[MODBUS_RTU_TXBUFFER_MAX];
            uint8_t                 Buffer[MODBUS_RTU_TXBUFFER_MAX];

            ModbusException         CurrentException = MBUS_EXCEPT_NONE;
            ModbusFunction          CurrentFunction = MBUS_FUNC_None;

            volatile uint16_t       BytesToSend = 0;
            volatile uint16_t       BytesToReceive = 0;

            void                    PollMaster();
            void                    PollSlave();

            void                    SetLastAddress(uint8_t Address, uint16_t Start);

            static void             ClearRXBuffer(ModbusRTU *);
            void                    ProcessFrame();

            static bool             VerifyRXCRC(ModbusRTU *);

            void                    SetState(ModbusState);

            void ExecuteMasterFunction(ModbusFunction Function, uint8_t Address, uint16_t Start, uint16_t Count);
            ModbusError InvokeTransmit(uint8_t, ModbusFunction, uint8_t *, uint16_t);

            void Transmit();
            void ResetSilenceCounter();
            bool CheckState(uint32_t *LastTick, uint32_t TickDuration);

            /*
                Handles Events:
                    - OnReadCoilsReturn     ->  (01) Modbus Read Coils
                                            ->  (02) Read Discrete Inputs
                    - OnReadRegisterReturn  ->  (03) Read Holding Registers
                                            ->  (04) Read Input Registers
                    - OnWriteCoilReturn     ->  (05) Write Single Coil
                    - OnWriteRegisterReturn ->  (06) Write Single Register

                    - OnExceptionRaised     ->  (80) All Exceptions
            */
            //Read Event Handlers
            void (*OnReadCoilsReturn)(ModbusRTU *, ModbusCoil *) = 0;
            void (*OnReadRegisterReturn)(ModbusRTU *, ModbusRegister *) = 0;

            //Exception Event Handler
            void (*OnExceptionRaised)(ModbusRTU *, ModbusExceptionEvent *) = 0;

            //Write Event Handler
            void (*OnWriteCoilReturn)(ModbusRTU *, ModbusCoil *) = 0;
            void (*OnWriteRegisterReturn)(ModbusRTU *, ModbusRegister *) = 0;

            #if MODBUS_RTU_DEBUG_MESSAGES == 1
                void PrintState();
            #endif

            bool GenerateCoilStruct(uint8_t, uint8_t, uint8_t, ModbusCoil *);
            bool GenerateRegisterStruct(uint8_t, uint8_t , uint8_t, ModbusRegister *);

            bool GenerateResponseCoilWriteStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusCoil * Coil);
            bool GenerateResponseRegisterWriteStruct(uint8_t Length, uint8_t Start, uint8_t End, ModbusRegister * Register);

            #if MODBUS_USE_WATCH_DOG ==1
                void RefreshWatchDog();
            #endif
    };
#endif
