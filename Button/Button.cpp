#include "Button.h"

Button::Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin){
    Port = ButtonPort;
    Pin = ButtonPin;
}
Button::Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnPress)(Button *)){
    Port = ButtonPort;
    Pin = ButtonPin;
    this->OnPressEvent = OnPress;
    Flags |= BUTTON_FLAG_RETURNS_ISPRESSED;
}
Button::Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnPress)(Button *), void (*OnLongPress)(Button *)){
    Port = ButtonPort;
    Pin = ButtonPin;
    this->OnPressEvent = OnPress;
    this->OnLongPressEvent = OnLongPress;
    Flags |= BUTTON_FLAG_RETURNS_ISPRESSED | BUTTON_FLAG_RETURNS_LONGPRESS;
}
Button::Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnDown)(Button *), void (*OnPress)(Button *), void (*OnLongPress)(Button *)){
    Port = ButtonPort;
    Pin = ButtonPin;
    this->OnPressEvent = OnPress;
    this->OnLongPressEvent = OnLongPress;
    this->OnDownEvent = OnDown;
    Flags |= BUTTON_FLAG_RETURNS_ISPRESSED | BUTTON_FLAG_RETURNS_LONGPRESS | BUTTON_FLAG_RETURNS_ONTOUCH;
}
Button::Button(GPIO_TypeDef * ButtonPort, uint16_t ButtonPin, void (*OnDown)(Button *), void (*OnPress)(Button *), void (*OnLongPress)(Button *), void (*OnUp)(Button *)){
    Port = ButtonPort;
    Pin = ButtonPin;
    this->OnPressEvent = OnPress;
    this->OnLongPressEvent = OnLongPress;
    this->OnDownEvent = OnDown;
    this->OnUpEvent = OnUp;
    Flags |= BUTTON_FLAG_RETURNS_ISPRESSED | BUTTON_FLAG_RETURNS_LONGPRESS | BUTTON_FLAG_RETURNS_ONTOUCH | BUTTON_FLAG_RETURNS_ONUPEVENT;
}

void Button::Invert(bool InvertState){
    if (InvertState ==  true){ OnState = GPIO_PIN_RESET; }
    else{ OnState = GPIO_PIN_SET;}
}
void Button::SetDebounce(uint16_t Length){
    this->DebounceWait = Length;
}

uint8_t Button::GetPresses(){
    return Presses;
}
void Button::SetLongPress(uint16_t Length){
    LongPress = Length;
}
void Button::SetResetPressCounterTime(uint16_t Length){
    MultiplePressReset = Length;
}

void Button::Invalidate(){
    if (HAL_GPIO_ReadPin(Port, Pin) == OnState){
        if ((HAL_GetTick()- DebounceTick)>DebounceWait){
            if ((Flags&BUTTON_FLAG_ISPRESSED) != BUTTON_FLAG_ISPRESSED){
                //Evaluate Pressed Event
                Presses++;
                PressTick = HAL_GetTick();
                Flags|=BUTTON_FLAG_ISPRESSED;
                InvokePressedEvent();
            }
            else{
                //Evaluate long presses
                if ((Flags&BUTTON_FLAG_ISHELD_EVALUATED)!=BUTTON_FLAG_ISHELD_EVALUATED){
                    if ((HAL_GetTick()-PressTick)>=LongPress){
                        Flags &= ~BUTTON_FLAG_ISPRESSED_CHECK;
                        Flags |= BUTTON_FLAG_ISHELD_EVALUATED;
                        InvokeHeldPressedEvent();
                    }
                }
            }
        }
    }
    else{ 
        DebounceTick = HAL_GetTick(); 
        Flags &=~ BUTTON_FLAG_ISPRESSED;
        Flags &=~ BUTTON_FLAG_ISHELD_EVALUATED;
        if ((Flags&BUTTON_FLAG_ISPRESSED_CHECK)==BUTTON_FLAG_ISPRESSED_CHECK){
            InvokePressedEvent();
        }
        if ((HAL_GetTick()-PressTick)>=MultiplePressReset){
            //Flags &= ~BUTTON_FLAG_ISPRESSED_CHECK;
            Presses = 0;
            if ((Flags&BUTTON_FLAG_RETURNS_ONUPEVENT) == BUTTON_FLAG_RETURNS_ONUPEVENT){
                if (OnUpEvent != NULL){ OnUpEvent(this);}
            }
        }
    }
}

void Button::InvokePressedEvent(){
    if ((Flags&BUTTON_FLAG_RETURNS_ISPRESSED)!=BUTTON_FLAG_RETURNS_ISPRESSED){return;}
    if (OnPressEvent == NULL){return;}
    if (OnDownEvent != NULL){
        OnDownEvent(this);
    }
    if ((Flags&BUTTON_FLAG_RETURNS_LONGPRESS)==BUTTON_FLAG_RETURNS_LONGPRESS){
        //Long Press Enabled
        if ((Flags&BUTTON_FLAG_ISPRESSED_CHECK)!=BUTTON_FLAG_ISPRESSED_CHECK){
            //Check if long press is in action first!
            Flags |= BUTTON_FLAG_ISPRESSED_CHECK;
            return;
        }
        else{
            Flags &= ~BUTTON_FLAG_ISPRESSED_CHECK;
            OnPressEvent(this);
        }
    }
    else{ OnPressEvent(this);}
}
void Button::InvokeHeldPressedEvent(){
    if ((Flags&BUTTON_FLAG_RETURNS_LONGPRESS)!=BUTTON_FLAG_RETURNS_LONGPRESS){return;}
    if (OnLongPressEvent == NULL){return;}
    OnLongPressEvent(this);
}