#include "ValueTable.h"
#include "stdio.h"
ValueTable::ValueTable(uint8_t Devices){
    this->Channels = 1;
    this->Devices = Devices;
    this->DeviceTable = (SensorDevice*)malloc((Devices) * sizeof(SensorDevice));
    this->ChannelData = (SensorChannel *)malloc(1 * sizeof(SensorChannel));
    this->Count = Devices;
    InitaliseTable();
}
ValueTable::ValueTable(uint8_t Channels, uint8_t Devices){
    this->Channels = Channels;
    this->Devices = Devices;
    this->DeviceTable = (SensorDevice*)malloc((Channels * Devices) * sizeof(SensorDevice));
    this->ChannelData = (SensorChannel *)malloc(Channels * sizeof(SensorChannel));
    this->Count = (Channels * Devices);
    InitaliseTable();
}
ValueTable::ValueTable(uint8_t Channels, uint8_t Devices, bool ForceState){
    this->Channels = Channels;
    this->Devices = Devices;
    this->DeviceTable = (SensorDevice*)malloc((Channels * Devices) * sizeof(SensorDevice));
    this->ChannelData = (SensorChannel *)malloc(Channels * sizeof(SensorChannel));
    this->Count = (Channels * Devices);
    InitaliseTable();
    this->ForceStateChangeRegister = ForceState;
}

ValueTable::~ValueTable(){
    free(this->DeviceTable);
    free(this->ChannelData);
}
void ValueTable::InitaliseTable(){
	for(int i=0;i<Channels;i++){
		ChannelData[i].UseName = false;
		ChannelData[i].Name = new String("");
	}
	for(int i=0;i<Count;i++){
		DeviceTable[i].Name = new String("");
	}
}

void ValueTable::ApplyName(uint8_t Device, String Name, SensorType Type){
    ApplyName(0, Device, Name, Type);
}
void ValueTable::ApplyName(uint8_t Channel, uint8_t Device, String Name, SensorType Type){
    uint16_t Index = GetIndex(Channel, Device);
    DeviceTable[Index].Name = new String(Name);
    DeviceTable[Index].Type = Type;
    //*DeviceTable[Index].Name = Name;
}

void ValueTable::SetChannelName(uint8_t Channel, String Name){
    if (Channel < Channels){
        ChannelData[Channel].Name = new String(Name);
        ChannelData[Channel].UseName = true;
    }
}

void ValueTable::Set(uint8_t Device, String Value){
    float Temp = Value.toFloat();
    Set(0, Device, Temp);
}
void ValueTable::Set(uint8_t Device, float Value){
    Set(0, Device, Value);
}

void ValueTable::Set(uint8_t Channel, uint8_t Device, String Value){
    float Temp = Value.toFloat();
    Set(Channel, Device, Temp);
}
void ValueTable::Set(uint8_t Channel, uint8_t Device, float Value){
    int32_t Index = GetIndex(Channel, Device);
    if (Index == -1){return;}
    if (DeviceTable[Index].Type != SensorType::Float){return;}
    bool ValueChanged = ForceStateChangeRegister;
    float PreviousValue = 0.0f;
    Get(Channel, Device, &PreviousValue);
    
    if (PreviousValue != Value){ValueChanged = true;}
    union {
        int32_t True;
        float Original;
    } Temp;
    Temp.Original = Value;
    DeviceTable[Index].Value = Temp.True;
    DeviceTable[Index].ValueChanged = ValueChanged;
}
void ValueTable::Set(uint8_t Channel, uint8_t Device, int32_t Value){
    int32_t Index = GetIndex(Channel, Device);
    if (Index == -1){return;}
    int32_t PreviousValue = 0; 
    Get(Channel, Device, &PreviousValue);
    bool ValueChanged = ForceStateChangeRegister;
    if (PreviousValue != Value){ValueChanged = true;}
    DeviceTable[Index].Value = Value;
    DeviceTable[Index].ValueChanged = ValueChanged;
}
void ValueTable::Get(uint8_t Device, float * Out){
    return Get(0, Device, Out);
}
void ValueTable::Get(uint8_t Channel, uint8_t Device, float * Out){
    int32_t Index = GetIndex(Channel, Device);
    if (Out == NULL){return;}
    if (Index == -1){
        *Out = 0.0f;
    }
    union {
        float FloatVal;
        int32_t Int32;
    } Group;
    Group.Int32 = DeviceTable[Index].Value;
    *Out = Group.FloatVal;
}
void ValueTable::Get(uint8_t Device, int32_t * Out){
    return Get(0, Device, Out);
}
void ValueTable::Get(uint8_t Channel, uint8_t Device, int32_t * Out){
    int32_t Index = GetIndex(Channel, Device);
    if (Out == NULL){return;}
    if (Index == -1){
        *Out = 0;
    }
    *Out = DeviceTable[Index].Value;
}

int32_t ValueTable::GetIndex(uint8_t Channel, uint8_t Device){
    if (Channel >= Channels){return -1;}
    if (Device >= Devices){return -1;}
    return (Channel * Devices) + Device;
}

bool ValueTable::GetState(uint8_t Channel, uint8_t Device){
    int32_t Index = GetIndex(Channel, Device);
    if (Index == -1){return false;}
    return DeviceTable[Index].ValueChanged;
}
void ValueTable::ForceStateChange(bool State){
    ForceStateChangeRegister = State;
}

void ValueTable::BuildCommandString(uint8_t Channel, uint8_t Device, String * Command){
    String * Output = Command;
    int32_t Index = GetIndex(Channel, Device);
    if (Index >=0){
        String OutputValue = "";
        if (DeviceTable[Index].Type == SensorType::Float){
            union{
                int32_t OldValue;
                float NewValue;
            } Change;
            Change.OldValue = DeviceTable[Index].Value;
            OutputValue = String(Change.NewValue);
        }
        else{
            OutputValue = String((long)DeviceTable[Index].Value);
        }
        DeviceTable[Index].ValueChanged = false;
        bool IncludeChannelName = ChannelData[Channel].UseName;
        if (IncludeChannelName == false){
            *Output = *DeviceTable[Index].Name + '=' + OutputValue;
        }
        else{
        	if (*ChannelData[Channel].Name != NULL){
        		*Output = (*ChannelData[Channel].Name) + (*DeviceTable[Index].Name) + '=' + OutputValue;
        	}
        	else{
        		*Output = *DeviceTable[Index].Name + '=' + OutputValue;
            }
        }
    }

}

#if VALUETABLE_DEBUG_PRINT == 1
    void ValueTable::PrintTable(){
        for(int y =0;y<Channels;y++){
            printf("CH %2d: ",y);
            for(int x =0;x<Devices;x++){
                printf("%04.4f ", Get(y,x));
            }
            printf("\n");
        }
    }
#endif

void ValueTable::GetArraySize(SensorDevice * DeviceTable, uint8_t * Channels, uint8_t * Devices){
    *Devices = this->Devices;
    *Channels = this->Channels;
    DeviceTable = this->DeviceTable;
}
