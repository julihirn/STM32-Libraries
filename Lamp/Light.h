#ifndef __LIGHT_H
    #define __LIGHT_H

    #include "Driver.h"
    #include "Generics.h"
    #include "LightFlags.h"
    #include "Math.h"

    struct ColorChannel{
        TIM_HandleTypeDef * Handle;
        uint32_t Channel;
    };
    struct RGB{
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };
    struct HSV{
        uint16_t H;
        uint8_t S;
        uint8_t V;
    };
    
    class Light{
        public:
            Light();
            Light(ColorChannel *);
            Light(ColorChannel *, ColorChannel *);
            Light(ColorChannel *, ColorChannel *, ColorChannel *);

            ~Light();
            void Set(uint8_t);
            void Set(uint8_t, uint8_t);
            void Set(uint8_t, uint8_t, uint8_t);

            void Set(RGB *);
            void Set(HSV *);

            void Brightness(uint8_t);
            void Set(bool);

        private:
            uint8_t Flags = 0x00;
            uint8_t SetBrightness = 0xFF;
            RGB Color;
            
            #if LAMP_USE_SCALE == 1
                uint16_t Resolution = 65535;
            #endif

            void Invalidate(void);
            void FromRgb(uint8_t, uint8_t, uint8_t);

            ColorChannel R;
            ColorChannel G;
            ColorChannel B;
    };
#endif