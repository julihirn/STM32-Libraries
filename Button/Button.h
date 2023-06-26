#ifndef __BUTTON_H
    #define __BUTTON_H

    #include "Driver.h"
    #include "ButtonFlags.h"

    class Button{
        public:
            Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin);
            Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnPress)(Button *));
            Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnPress)(Button *), void (*OnLongPress)(Button *));
            Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnDown)(Button *), void (*OnPress)(Button *), void (*OnLongPress)(Button *));
            Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnDown)(Button *), void (*OnPress)(Button *), void (*OnLongPress)(Button *), void (*OnUp)(Button *));
            void Invert(bool);
            void SetDebounce(uint16_t Length);
            void SetLongPress(uint16_t Length);
            void SetResetPressCounterTime(uint16_t Length);

            ~Button();
            
            uint8_t GetPresses();
            bool IsButtonDown();
            void Invalidate();
        private:
            GPIO_TypeDef * Port;
            uint16_t Pin;

            uint16_t Flags = 0;

            uint8_t Presses = 0;

            uint16_t MultiplePressReset = 500;
            uint16_t LongPress = 1000;

            GPIO_PinState OnState = GPIO_PIN_SET;

            void InvokePressedEvent();
            void InvokeHeldPressedEvent();
            
            //Tick Keepers
            uint16_t DebounceWait = 10;
            uint32_t DebounceTick = 0;
            uint32_t PressTick = 0;

            //Event Handlers
            void (*OnDownEvent)(Button *) = 0;
            void (*OnPressEvent)(Button *) = 0;
            void (*OnLongPressEvent)(Button *) = 0;
            void (*OnUpEvent)(Button *) = 0;
    };
#endif