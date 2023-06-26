#include "PWM.h"

PWM::PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax){
    DefineTimer(TmrHandle, TmrChannel, PWMMax);
    Set(0x00);
}
PWM::PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty){
    DefineTimer(TmrHandle, TmrChannel, PWMMax);
    this->InvertDuty(InvertDuty);
    Set(0x00);
}
PWM::PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, GPIO_TypeDef * EnbPort, uint16_t EnbPin){
    DefineTimer(TmrHandle, TmrChannel, PWMMax);
    DefineEnable(EnbPort, EnbPin);
    Enable(false);
    Set(0x00);
}
PWM::PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty, GPIO_TypeDef * EnbPort, uint16_t EnbPin){
    DefineTimer(TmrHandle, TmrChannel, PWMMax);
    this->InvertDuty(InvertDuty);
    DefineEnable(EnbPort, EnbPin);
    Enable(false);
    Set(0x00);
}
PWM::PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty, bool InvertEnable, GPIO_TypeDef * EnbPort, uint16_t EnbPin){
    DefineTimer(TmrHandle, TmrChannel, PWMMax);
    this->InvertDuty(InvertDuty);
    DefineEnable(EnbPort, EnbPin);
    this->InvertEnable(InvertEnable);
    Enable(false);
    Set(0x00);
}

void PWM::DefineTimer(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax){
    Handle = TmrHandle;
    Channel = TmrChannel;
    MaxSpeed = PWMMax;
}
void PWM::DefineEnable(GPIO_TypeDef * EnbPort, uint16_t EnbPin){
    this->EnablePort = EnbPort;
    this->EnablePin = EnbPin;
    DeviceFlags |= PWM_FLAG_HASENABLE | PWM_FLAG_HASENPIN;
}

void PWM::Set(float SetSpeed, bool IsPercentage){
    if (IsPercentage == true){SetSpeed/=100.0f;}
    if (SetSpeed < 0.0f){SetSpeed = 0.0f;}
    if (SetSpeed > 1.0f){SetSpeed = 1.0f;}
    uint32_t TempSpeed = (uint32_t)(SetSpeed * (float)MaxSpeed);
    Set(TempSpeed);
}
void PWM::Set(uint32_t SetSpeed){
    uint32_t TempSpeed = SetSpeed;
    if (CheckFlag(PWM_FLAG_DUTYINVERT) == true){
        if (SetSpeed > MaxSpeed){TempSpeed = 0;}
        else{TempSpeed = MaxSpeed - SetSpeed;}
    }
    if (TempSpeed >= MaxSpeed){Speed = MaxSpeed;}
    if (TempSpeed == 0){
        Speed = 0;
        Halt();
    }
    else{
    	Speed = TempSpeed;
        SetInstance(Speed);
        DeviceFlags |= PWM_FLAG_INMOTION;
    }
}
void PWM::Enable(bool Set){
    if (CheckFlag(PWM_FLAG_HASENABLE) == false){return;}
    
    Enabled = Set;
    if (EnablePort == NULL){return;}
    if (CheckFlag(PWM_FLAG_HASENPIN) == true){
        GPIO_PinState State = GPIO_PIN_RESET;
        if (CheckFlag(PWM_FLAG_ENINVERT) == true){
            State = (Enabled == true?GPIO_PIN_RESET:GPIO_PIN_SET);
        }
        else{ State = (Enabled == true?GPIO_PIN_SET:GPIO_PIN_RESET);}
        HAL_GPIO_WritePin(EnablePort, EnablePin, State);
    }
}

void PWM::Halt(){
	HAL_TIM_PWM_Stop(Handle, Channel);
    SetInstance(0);
    DeviceFlags &= ~PWM_FLAG_INMOTION;
}

void PWM::SetInstance(uint32_t PWMSpeed){
    switch(Channel){
        case TIM_CHANNEL_1:
            Handle->Instance->CCR1 = PWMSpeed; break;
        case TIM_CHANNEL_2:
            Handle->Instance->CCR2 = PWMSpeed; break;
        case TIM_CHANNEL_3:
            Handle->Instance->CCR3 = PWMSpeed; break;
        case TIM_CHANNEL_4:
            Handle->Instance->CCR4 = PWMSpeed; break;
        default:
            return;
    }
    HAL_TIM_PWM_Start(Handle, Channel);
}

void PWM::InvertEnable(bool Invert){
    if (Invert == true){ DeviceFlags |=PWM_FLAG_ENINVERT;}
    else{ DeviceFlags &=~PWM_FLAG_ENINVERT;}
}
void PWM::InvertDuty(bool Invert){
    if (Invert == true){ DeviceFlags |=PWM_FLAG_DUTYINVERT;}
    else{ DeviceFlags &=~PWM_FLAG_DUTYINVERT;}
}
bool PWM::CheckFlag(uint16_t Flag){
    if((DeviceFlags&Flag)==Flag){ return true;}
    return false;
}
