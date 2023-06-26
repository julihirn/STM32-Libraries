#ifndef _SENSORTABLE_H
    #define _SENSORTABLE_H

    #include "Driver.h"
    #include "Generics.h"
    enum class SensorType{
        Int32 = 0x00,
        Float = 0x01
    };
    struct SensorDevice{
        SensorType Type;
        int32_t Value;
        bool ValueChanged;
        String * Name;
    };
    struct SensorChannel{
        bool UseName;
        String * Name;
    };
    class ValueTable{
        public:

            ValueTable(uint8_t Devices);
            ValueTable(uint8_t Channels, uint8_t Devices);
            ValueTable(uint8_t Channels, uint8_t Devices, bool ForceState);
            ~ValueTable();

            void ApplyName(uint8_t Device, String Name, SensorType Type);
            void ApplyName(uint8_t Channel, uint8_t Device, String Name, SensorType Type);

            void SetChannelName(uint8_t Channel, String Name);

            void Set(uint8_t Device, String Value);
            void Set(uint8_t Device, float Value);
            void Set(uint8_t Device, int32_t Value);

            void Set(uint8_t Channel, uint8_t Device, String Value);
            void Set(uint8_t Channel, uint8_t Device, float Value);
            void Set(uint8_t Channel, uint8_t Device, int32_t Value);

            void ForceStateChange(bool);
            
            void Get(uint8_t Device, float * Out);
            void Get(uint8_t Channel, uint8_t Device, float * Out);
            void Get(uint8_t Device, int32_t * Out);
            void Get(uint8_t Channel, uint8_t Device, int32_t * Out);

            bool GetState(uint8_t Channel, uint8_t Device);

            void BuildCommandString(uint8_t Channel, uint8_t Device, String * Command);

            #if VALUETABLE_DEBUG_PRINT == 1
                void PrintTable();
            #endif

            void GetArraySize(SensorDevice *, uint8_t *, uint8_t *);
        private:
            SensorDevice * DeviceTable;
            SensorChannel * ChannelData;
            uint8_t Channels = 1;
            uint8_t Devices = 1;
            uint16_t Count = 1;

            bool ForceStateChangeRegister = false;

            void InitaliseTable();

            int32_t GetIndex(uint8_t Channel, uint8_t Device);
    };
#endif
