#ifndef _WS2812B_H
    #define _WS2812B_H
	
	#include "Driver.h"
	#include "Generics.h"
	#include "WS2812BLookUps.h"
	
	typedef union{
		struct{
			uint8_t b;
			uint8_t r;
			uint8_t g;
		} color;
		uint32_t data;
	} PixelRGB_t;

	struct HSV{
	    uint16_t h;
	    double s;       // a fraction between 0 and 1
	    double v;       // a fraction between 0 and 1
	};
	#define NEOPIXEL_ZERO 77//25 //(ARR+1)(0.32) --ROUND NUMBER
	#define NEOPIXEL_ONE 154//67 //(ARR+1)(0.64) --ROUND NUMBER
	class WS2812B{
	    public:
	        WS2812B(TIM_HandleTypeDef * Handle, uint32_t Channel, uint16_t Pixels);
	        void SetColor(uint16_t, PixelRGB_t *);
	        void SetColor(uint16_t, uint32_t);
	        void SetColor(uint16_t, uint8_t, uint8_t, uint8_t);
	        void SetColor(uint16_t, HSV *);
	
	        void Invalidate();
	
	        void SetBrightness(uint8_t);
			void SetBrightness(uint16_t);
			void On();
			void Off();

			void Clear(bool InvalidateAll);
			void BlankPixel(uint16_t Pixel);
	
	        static void Hsv2Rgb(HSV * In, PixelRGB_t * Out);
	        static void Rgb2Hsv(PixelRGB_t * In, HSV * Out);
	    private:
			uint32_t * PxBuff = NULL;
	        PixelRGB_t * Colors;
	        //uint32_t * DMABuffer;
			uint32_t * DMABuffer;
	        uint16_t DMABufferSize;
	        uint16_t PixelsCount;
			bool IsOn = true;
	
	        TIM_HandleTypeDef * Handle;
	        uint32_t Channel;
	
	        float Brightness = 1;
	
	        void AdjustColor(PixelRGB_t * In, PixelRGB_t * Out);
	        
	        static float max3(float a, float b, float c);
	        static float min3(float a, float b, float c);
	};
#endif