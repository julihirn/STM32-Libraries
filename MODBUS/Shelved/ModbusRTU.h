#ifndef _MODBUS_RTU_H
    #define _MODBUS_RTU_H

    #include "Modbus.h"

    class ModbusRTU{
        public:
            ModbusError Initalise(uint8_t, uint8_t, uint32_t, ModbusParity);

            ModbusError Close();
            ModbusError Enable();
            ModbusError Disable();
            ModbusError MasterDisable();

            ModbusError Poll();
            ModbusError MasterPoll(void);

            void SetBits(uint8_t *, uint16_t, uint8_t, uint8_t);
            uint8_t GetBits(uint8_t *, uint16_t, uint8_t);


            bool RTUReceiveFSM(void);
            bool RTUTransmitFSM(void);
            bool MasterRTUTransmitFSM(void);
            bool RTUTimerExpired(void);
            bool RTUMasterTimerExpired(void);
            
        private:
            //Handles
            UART_HandleTypeDef * hndl_comms;
            TIM_HandleTypeDef * hndl_timer;

            uint16_t Timeout = 0;
            uint16_t DownCounter = 0;

            uint8_t    ucMBAddress;
            ModbusSystemState SystemState = STATE_NOT_INITIALIZED; 
            //eMBMode  eMBCurrentMode;

            volatile bool xFrameIsBroadcast = false;

            bool InMasterMode = false;

            bool xEventInQueue;
            ModbusState eQueuedEvent;

            ModbusState CurrentMasterStatus;

            MasterSetTimerMode TimerMode;

            bool InitaliseTimer(uint16_t);
            void EnableTimer();
            void DisableTimer();

            void MasterSetTimeMode(MasterSetTimerMode eMBTimerMode);

            bool MasterIsEstablished();

            void CommunicationsInterrupts(bool, bool);
            bool CommunicationsInitalise(uint8_t, uint32_t, uint8_t, ModbusParity);
            bool TransmitByte(uint8_t);
            bool ReceiveByte(uint8_t *);

            void RTUStart();
            void RTUStop();

            ModbusError RTUReceive(uint8_t *, uint8_t **, uint16_t *);
            ModbusError RTUSend(uint8_t, const uint8_t *, uint16_t);

            

            void MasterGetRTUTXBuffer(uint8_t **);
            void MasterGetPDUTXBuffer(uint8_t **);
            void MasterSetPDUTXLength(uint16_t);
            uint16_t MasterGetPDUTXLength(void);
            bool MasterRequestIsBroadcast(void);

            void RunInMasterMode(bool IsMasterMode);
            bool IsMasterMode(void);

            bool EventInitalise();
            bool EventPost(ModbusState);
            bool EventRetrieve(ModbusState *);

            ModbusError RTUInitalise(uint8_t, uint8_t, uint32_t, ModbusParity);
            ModbusError MasterReqReadCoils(uint8_t, uint16_t, uint16_t, int32_t);

            ModbusState MasterGetErrorType(void);
            void MasterSetErrorType(ModbusState ErrorType);

            uint8_t MasterGetDestinationAddress(void);
            void MasterSetDestinationAddress(uint8_t);
            uint8_t ucMBMasterDestAddress = 0x00;

            volatile ModbusTXState TXState;
            volatile ModbusRXState RXState;

            volatile uint8_t  ucRTUBuf[MB_SER_PDU_SIZE_MAX];
            volatile uint8_t  ucMasterRTUSndBuf[MB_PDU_SIZE_MAX];
            volatile uint8_t  ucMasterRTURcvBuf[MB_SER_PDU_SIZE_MAX];
            volatile uint16_t usMasterSendPDULength;

            volatile uint8_t *pucSndBufferCur;
            volatile uint16_t usSndBufferCount;

            volatile uint16_t usRcvBufferPos;
    };
#endif
