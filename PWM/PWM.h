#ifndef _PWM_H
    #define _PWM_H

    #include "Driver.h"
    #include "Generics.h"
    #include "PWMFlags.h"

    //struct Connection{
    //    GPIO_TypeDef * Port;
    //    uint16_t Pin;
    //};
    class PWM{
        public:
            PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax);
            PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty);
            PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, GPIO_TypeDef * EnbPort, uint16_t EnbPin);
            PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty, GPIO_TypeDef * EnbPort, uint16_t EnbPin);
            PWM(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax, bool InvertDuty, bool InvertEnable, GPIO_TypeDef * EnbPort, uint16_t EnbPin);
            

            void Set(uint32_t SetSpeed);
            void Set(float  SetSpeed, bool IsPercentage);
            void Enable(bool Set);

            void Halt(void);

            void InvertDuty(bool Invert);
            void InvertEnable(bool Invert);

            uint32_t Speed = 0;
        private:
            bool Enabled = false;
            uint32_t MaxSpeed = 0;
            uint16_t DeviceFlags = 0x00;
            TIM_HandleTypeDef * Handle;
            uint32_t Channel;

            GPIO_TypeDef * EnablePort;
            uint16_t EnablePin;

            void DefineTimer(TIM_HandleTypeDef * TmrHandle, uint32_t TmrChannel, uint32_t PWMMax);
            void DefineEnable(GPIO_TypeDef * EnbPort, uint16_t EnbPin);

            void SetInstance(uint32_t PWMSpeed);
            bool CheckFlag(uint16_t Flag);
    };
#endif
