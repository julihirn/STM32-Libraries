#include "Pulse.h"

Pulse::Pulse(GPIO_TypeDef * Port, uint16_t Pin){
    this->Port = Port;
    this->Pin = Pin;
}

void Pulse::Invert(bool Invert){
    if(Invert == true){
        this->OnState = GPIO_PIN_RESET;
        this->OffState = GPIO_PIN_SET;
    }
    else{
        this->OnState = GPIO_PIN_SET;
        this->OffState = GPIO_PIN_RESET;
    }
    SetPort(false);
}
void Pulse::Trigger(){
    if (PulseWaitTime > 0){
        Flags |= PULSE_FLAG_ISWAITING; 
    }
    Flags |= PULSE_FLAG_ISACTIVE;
    PulseTick = HAL_GetTick();
    WaitTick = PulseTick;
}

void Pulse::SetOnLength(uint16_t Length){
    this->PulseOnTime = Length;
}
void Pulse::SetDelayLength(uint16_t Length){
    this->PulseWaitTime = Length;
}

void Pulse::Invalidate(void){
    if ((Flags&PULSE_FLAG_ISACTIVE)==PULSE_FLAG_ISACTIVE){
        if ((Flags&PULSE_FLAG_ISWAITING)==PULSE_FLAG_ISWAITING){
            if ((HAL_GetTick()-WaitTick)>=PulseWaitTime){
                Flags &=~PULSE_FLAG_ISWAITING;
                PulseTick = HAL_GetTick();
            }
        }
        else{
            if ((HAL_GetTick()-PulseTick)>=PulseOnTime){
                SetPort(false);
                Flags &=~PULSE_FLAG_ISACTIVE;
            }
            else{ SetPort(true);}
        }
    }
    else{SetPort(false);}
}
void Pulse::SetPort(bool On){
    if (On == true){ HAL_GPIO_WritePin(Port,Pin,OnState);}
    else{ HAL_GPIO_WritePin(Port,Pin,OffState);}
}