//typedef union{
//  struct{
//    uint8_t b;
//    uint8_t r;
//    uint8_t g;
//  } color;
//  uint32_t data;
//} PixelRGB_t;

//https://forum.digikey.com/t/controlling-neopixels-with-stm32/20527

#include "WS2812B.h"
#include <stdlib.h>

WS2812B::WS2812B(TIM_HandleTypeDef * Handle, uint32_t Channel, uint16_t Pixels){
    this->Handle = Handle;
    this->Channel = Channel;
    PixelsCount = Pixels;
    DMABufferSize = (24 * Pixels) + 1;
    DMABuffer = (uint32_t *)malloc(sizeof(uint32_t) * DMABufferSize);//(uint32_t *)malloc(DMABufferSize);
    Colors = (PixelRGB_t *)malloc(sizeof(PixelRGB_t) * Pixels);
    for(int i = 0;i<Pixels;i++){
        (*(Colors + i)).data = 0;
    }
    for(int i = 0;i<DMABufferSize;i++){
        *(DMABuffer + i) = 0;
    }
}
void WS2812B::SetColor(uint16_t Pixel, PixelRGB_t * Color){
    if (Pixel > PixelsCount){ return;}
    *(Colors + Pixel) = *Color;
}
void WS2812B::SetColor(uint16_t Pixel, uint32_t Color){
    if (Pixel > PixelsCount){ return;}
    (*(Colors + Pixel)).data = Color;
}
void WS2812B::SetColor(uint16_t Pixel, uint8_t R, uint8_t G, uint8_t B){
    if (Pixel > PixelsCount){ return;}
    PixelRGB_t Color;
    Color.color.r = R;
    Color.color.g = G;
    Color.color.b = B;
    *(Colors + Pixel) = Color;
}
void WS2812B::SetColor(uint16_t Pixel, HSV * Color){
    if (Pixel > PixelsCount){ return;}
    PixelRGB_t TempColor;
    Hsv2Rgb(Color, &TempColor);
    *(Colors + Pixel) = TempColor;
}
void WS2812B::Hsv2Rgb(HSV * In, PixelRGB_t * Out){
    double      hh, p, q, t, ff;
    long        i;
    if(In->s <= 0.0) {       // < is bogus, just shuts up warnings
        Out->color.r = In->v;
        Out->color.g = In->v;
        Out->color.b = In->v;
        return;
    }
    hh = In->h;
    if(hh >= 360.0) hh = 0.0;
    hh = Angledivide(hh);
    i = (long)hh;
    ff = hh - i;
    p = In->v * (1.0 - In->s);
    q = In->v * (1.0 - (In->s * ff));
    t = In->v * (1.0 - (In->s * (1.0 - ff)));
    switch(i) {
    case 0:
        Out->color.r = 255.0f*In->v;
        Out->color.g = 255.0f*t;
        Out->color.b = 255.0f*p;
        break;
    case 1:
        Out->color.r = 255.0f*q;
        Out->color.g = 255.0f*In->v;
        Out->color.b = 255.0f*p;
        break;
    case 2:
        Out->color.r = 255.0f*p;
        Out->color.g = 255.0f*In->v;
        Out->color.b = 255.0f*t;
        break;

    case 3:
        Out->color.r = 255.0f*p;
        Out->color.g = 255.0f*q;
        Out->color.b = 255.0f*In->v;
        break;
    case 4:
        Out->color.r = 255.0f*t;
        Out->color.g = 255.0f*p;
        Out->color.b = 255.0f*In->v;
        break;
    case 5:
    default:
        Out->color.r = 255.0f*In->v;
        Out->color.g = 255.0f*p;
        Out->color.b = 255.0f*q;
        break;
    }
    return; 
}
void WS2812B::Rgb2Hsv(PixelRGB_t * In, HSV * Out){
    // R, G, B values are divided by 255
    // to change the range from 0..255 to 0..1:
    float h, s, v;
    float r, g, b;
    r = (float)In->color.r;
    g = (float)In->color.g;
    b = (float)In->color.b;
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
    float cmax = max3(r, g, b); // maximum of r, g, b
    float cmin = min3(r, g, b); // minimum of r, g, b
    float diff = cmax-cmin; // diff of cmax and cmin.
    if (cmax == cmin){ h = 0;}
    else if (cmax == r){ h = fmod((60 * ((g - b) / diff) + 360), 360.0);}
    else if (cmax == g){ h = fmod((60 * ((b - r) / diff) + 120), 360.0);}
    else if (cmax == b){ h = fmod((60 * ((r - g) / diff) + 240), 360.0);}
    if (cmax == 0){  s = 0; }
    else {s = (diff / cmax);}
    // compute v
    v = cmax;
    Out->h = h;
    Out->s = s;
    Out->v = v;
}
void WS2812B::SetBrightness(uint8_t BrightnessSetting){
    Brightness = (float)BrightnessSetting / (float)0xFF;
}
void WS2812B::SetBrightness(uint16_t BrightnessSetting){
    Brightness = (float)BrightnessSetting / (float)0xFFFF;
}
void WS2812B::Invalidate(){
    PxBuff = DMABuffer;
    for(int i = 0; i<PixelsCount;i++){
        PixelRGB_t * Current = Colors+i;
        PixelRGB_t Adjusted;
        AdjustColor(Current, &Adjusted);
        for (int j=23; j>=0;j--){
            if ((Adjusted.data >> j) & 0x01){
                *PxBuff = NEOPIXEL_ONE;
            }
            else{ *PxBuff = NEOPIXEL_ZERO;}
            PxBuff++;
        }
    }
    *(DMABuffer + DMABufferSize - 1) = 0; // last element must be 0!
    HAL_TIM_PWM_Start_DMA(Handle, Channel, DMABuffer, DMABufferSize);
    HAL_Delay(10);
}
void WS2812B::AdjustColor(PixelRGB_t * In, PixelRGB_t * Out){
    if (IsOn == true){
        Out->color.r = (uint8_t)((float)In->color.r * Brightness);
        Out->color.g = (uint8_t)((float)In->color.g * Brightness);
        Out->color.b = (uint8_t)((float)In->color.b * Brightness);
    }
    else{
        Out->color.r = 0;
        Out->color.g = 0;
        Out->color.b = 0;
    }
}

void WS2812B::On(){IsOn=true;Invalidate();}
void WS2812B::Off(){IsOn=false;Invalidate();}

void WS2812B::Clear(bool InvalidateAll = true){
    PixelRGB_t OffColor;
    OffColor.color.r = 0; OffColor.color.g = 0; OffColor.color.b = 0; 
    for (int i =0;i<PixelsCount;i++){
        *(Colors + i) = OffColor;
    }
    if (InvalidateAll == true){
        Invalidate();
    }
}
void WS2812B::BlankPixel(uint16_t Pixel){
    if (Pixel >= PixelsCount){ return;}
    PixelRGB_t OffColor;
    OffColor.color.r = 0; OffColor.color.g = 0; OffColor.color.b = 0; 
    *(Colors + Pixel) = OffColor;
}
float WS2812B::max3(float a, float b, float c) {
   return ((a > b)? (a > c ? a : c) : (b > c ? b : c));
}
float WS2812B::min3(float a, float b, float c) {
   return ((a < b)? (a < c ? a : c) : (b < c ? b : c));
}
