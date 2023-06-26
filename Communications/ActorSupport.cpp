#include "Actor.h"

bool ActorSupport::GetBoolean(Actor * Input){
    bool Temp = false;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
char ActorSupport::GetChar(Actor * Input){
    char Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
uint8_t ActorSupport::GetUint8(Actor * Input){
    uint8_t Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
int8_t ActorSupport::GetInt8(Actor * Input){
    int8_t Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
int ActorSupport::GetInt(Actor * Input){
    int Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
uint16_t ActorSupport::GetUint16(Actor * Input){
    uint16_t Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
int16_t ActorSupport::GetInt16(Actor * Input){
    int16_t Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}
float ActorSupport::GetFloat(Actor * Input){
    float Temp = 0x00;
    if (Input == NULL){return Temp;}
    Input->GetData(&Temp);
    return Temp;
}