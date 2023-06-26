#include "max7317.h"


MAX7317::MAX7317(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin){
    this->Handle = Handle;
    this->Pin = Pin;
    this->Port = Port;
    Deselect();
}
void MAX7317::Deselect(){
	if (Port != NULL){
		HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
	}
}
void MAX7317::PushBuffer(){
    Push(Buffer);
}
void MAX7317::Push(uint16_t State){
    uint16_t Temp = 0x3FF & State;
    if (Temp == 0x0000){ SetAll(false); return; }
    else if (Temp == 0x03FF){ SetAll(true); return; }
    for(int i=0;i<10;i++){
        uint16_t Shift = 0x01<<i;
        if ((Temp&Shift)==Shift){
            Set(i, true);
        }
        else{Set(i, false);}
        HAL_Delay(1);
    }
}
void MAX7317::InvertAll(bool Invert){
    InvertSets = Invert;
}
void MAX7317::Set(uint8_t OutputPort, bool State){
    if (Port == NULL){ return;}
    if (Handle == NULL){return;}
    if (OutputPort >= 10){return;}
    uint8_t TxBuffer = 0x00;
    MAX7317Command Command = (MAX7317Command)OutputPort;
    bool StateBuffer = State;
    uint16_t DataByte = 0x01 << OutputPort;
    if (InvertSets == true){StateBuffer = !State;}
    if (StateBuffer == true){
        TxBuffer = 0x01;
        this->Buffer |=DataByte;
    }   
    else{
        this->Buffer &=~DataByte;
    }
    Write(Command, TxBuffer);
}
void MAX7317::SetAll(bool State){
    if (Port == NULL){ return;}
    if (Handle == NULL){return;}
    uint8_t TxBuffer = 0x00;
    uint16_t DataByte = 0x03FF;
    bool StateBuffer = State;
    if (InvertSets == true){StateBuffer = !State;}
    if (StateBuffer == true){
        TxBuffer = 0x01;
        this->Buffer |=DataByte;
    }   
    else{
        this->Buffer &=~DataByte;
    }
    Write(MAX7317Command::WriteAllPorts, TxBuffer);
}
uint16_t MAX7317::Read(void){
    if (Port == NULL){ return 0x00;}
    if (Handle == NULL){return 0x00;}
    HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_RESET);
    union {
        uint16_t Output;
        uint8_t Bytes[2];
    } Value;
    //uint8_t Buffer = 0x00;
    Write(MAX7317Command::ReadPorts0_7, 0x00);
    HAL_Delay(1);
    Value.Bytes[0] = (0xFF00 & Receive(MAX7317Command::ReadPorts8_9)) >> 8;
    HAL_Delay(1);
    Value.Bytes[1] = 0x0007 & ((0xFF00 & Receive(MAX7317Command::ReadPorts8_9)) >> 8);
    return Value.Output;
}
void MAX7317::Write(MAX7317Command Command, uint8_t Data){
    uint8_t TxBuffer[2] = {0, 0};
    TxBuffer[0] = (uint8_t)Command;
    TxBuffer[1] = Data;
    uint16_t CommunicationLength = sizeof(uint8_t) * 2;
    HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(this->Handle, TxBuffer, CommunicationLength,1);
    HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
}
uint16_t MAX7317::Receive(MAX7317Command Command){
    uint8_t TxBuffer[2] = {0, 0};
    TxBuffer[0] = (uint8_t)Command;
    TxBuffer[1] = 0x00;
    //uint8_t RxBuffer[2] = {0, 0};
    union {
        uint16_t Output;
        uint8_t Bytes[2];
    } RxBuffer;
    RxBuffer.Output = 0;
    HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(this->Handle,TxBuffer,RxBuffer.Bytes,2*sizeof(uint8_t),1);
    HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
    return RxBuffer.Output;
}
