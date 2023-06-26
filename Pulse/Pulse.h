#ifndef _PULSE_H
    #define _PULSE_H

    #include "Driver.h"
    #include "PulseFlags.h"
    class Pulse{
        public:
            Pulse(GPIO_TypeDef * Port, uint16_t Pin);
            ~Pulse();

            void SetOnLength(uint16_t);
            void SetDelayLength(uint16_t);

            void Trigger(void);
            void Invert(bool);

            void Invalidate(void);
        private:
            GPIO_TypeDef * Port;
            uint16_t Pin;

            uint8_t Flags = 0;

            GPIO_PinState OnState = GPIO_PIN_SET;
            GPIO_PinState OffState = GPIO_PIN_RESET;

            void SetPort(bool);

            uint16_t PulseOnTime = 200;
            uint16_t PulseWaitTime = 0;

            uint32_t PulseTick = 0;
            uint32_t WaitTick = 0;
    };
#endif