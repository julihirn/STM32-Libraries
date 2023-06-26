#include "Light.h"


Light::Light(){
}
Light::Light(ColorChannel * R){
    this->R.Handle = R->Handle;
    this->R.Channel = R->Channel;
    Flags |= LAMP_HAS_CHAN_R;
}
Light::Light(ColorChannel * R, ColorChannel * G){
    this->R.Handle = R->Handle;
    this->R.Channel = R->Channel;

    this->G.Handle = G->Handle;
    this->G.Channel = G->Channel;
    Flags |= LAMP_HAS_CHAN_R | LAMP_HAS_CHAN_G;
}
Light::Light(ColorChannel * R, ColorChannel * G, ColorChannel * B){
    this->R.Handle = R->Handle;
    this->R.Channel = R->Channel;

    this->G.Handle = G->Handle;
    this->G.Channel = G->Channel;

    this->B.Handle = B->Handle;
    this->B.Channel = B->Channel;
    Flags |= LAMP_HAS_CHAN_R | LAMP_HAS_CHAN_G | LAMP_HAS_CHAN_B;
}


void Light::Set(bool IsOn){
    if (IsOn == true){
        Flags |= LAMP_IS_ON;
        if ((Flags & LAMP_HAS_CHAN_R) == LAMP_HAS_CHAN_R){
            HAL_TIM_PWM_Start(R.Handle, R.Channel);
        }
        if ((Flags & LAMP_HAS_CHAN_G) == LAMP_HAS_CHAN_G){
            HAL_TIM_PWM_Start(G.Handle, G.Channel);
        }
        if ((Flags & LAMP_HAS_CHAN_B) == LAMP_HAS_CHAN_B){
            HAL_TIM_PWM_Start(B.Handle, B.Channel);
        }
    }
    else{
        Flags &= ~LAMP_IS_ON;
        if ((Flags & LAMP_HAS_CHAN_R) == LAMP_HAS_CHAN_R){
            HAL_TIM_PWM_Stop(R.Handle, R.Channel);
        }
        if ((Flags & LAMP_HAS_CHAN_G) == LAMP_HAS_CHAN_G){
            HAL_TIM_PWM_Stop(G.Handle, G.Channel);
        }
        if ((Flags & LAMP_HAS_CHAN_B) == LAMP_HAS_CHAN_B){
            HAL_TIM_PWM_Stop(B.Handle, B.Channel);
        }
    }
}
void Light::Set(uint8_t R){
    this->Color.R = R;
    Invalidate();
}
void Light::Set(uint8_t R, uint8_t G){
    this->Color.R = R;
    this->Color.G = G;
    Invalidate();
}
void Light::Set(uint8_t R, uint8_t G, uint8_t B){
    this->Color.R = R;
    this->Color.G = G;
    this->Color.B = B;
    Invalidate();
}
void Light::Set(RGB *Color){
    this->Color.R = Color->R;
    this->Color.G = Color->G;
    this->Color.B = Color->B;
    Invalidate();
}
void Light::Set(HSV *Color){
    float s = (float)Color->S / 255.0f;
    float b = (float)Color->V / 255.0f;
    int hi = (int)(floorf((float)Color->H / 60.0f)) % 6;
    float f = (float)Color->H / 60.0f - floorf((float)Color->H / 60.0f);
    int val = 0;
    val = (int)(b * 255.0f);
    uint8_t v = (uint8_t)(val);
    uint8_t p = (uint8_t)(val * (1.0f - s));
    uint8_t q = (uint8_t)(val * (1.0f - f * s));
    uint8_t t = (uint8_t)(val * (1.0f - (1.0f - f) * s));
    if (hi == 0){ FromRgb(v, t, p);}
    else if (hi == 1){FromRgb(q, v, p);}
    else if (hi == 2){FromRgb(p, v, t);} 
    else if (hi == 3){FromRgb(p, q, v);}
    else if (hi == 4){FromRgb(t, p, v);}
    else{FromRgb(v, p, q);}
    Invalidate();
}
void Light::FromRgb(uint8_t R, uint8_t G, uint8_t B){
    this->Color.R = R;
    this->Color.G = G;
    this->Color.B = B;
}
void Light::Brightness(uint8_t Level){
    SetBrightness = Level;
    Invalidate();
}

void Light::Invalidate(){
    float BrightnessScale = (float)SetBrightness/255.0f;
    
    if ((Flags & LAMP_HAS_CHAN_R) == LAMP_HAS_CHAN_R){
        #if LAMP_USE_SCALE == 1
            uint16_t RC = Resolution * ((float)Color.R / 255.0f) * BrightnessScale;
        #else
            uint16_t RC = Color.R * BrightnessScale;
        #endif
        R.Handle->Instance->CCR1 = RC;
    }
    if ((Flags & LAMP_HAS_CHAN_G) == LAMP_HAS_CHAN_G){
        #if LAMP_USE_SCALE == 1
            uint16_t GC = Resolution * ((float)Color.G / 255.0f) * BrightnessScale;
        #else
            uint16_t GC = Color.G * BrightnessScale;
        #endif
        G.Handle->Instance->CCR1 = GC;
    }
    if ((Flags & LAMP_HAS_CHAN_B) == LAMP_HAS_CHAN_B){
        #if LAMP_USE_SCALE == 1
            uint16_t BC = Resolution * ((float)Color.B / 255.0f) * BrightnessScale;
        #else
            uint16_t BC = Color.B * BrightnessScale;
        #endif
        B.Handle->Instance->CCR1 = BC;
    }
}