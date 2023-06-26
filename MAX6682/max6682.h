#ifndef _MAX6682_H
	#define _MAX6682_H

	#include "Driver.h"
	#include "Generics.h"
	#if MAX6682_USE_SPIHANDLER == 1
		#include "SPIHandler.h"
	#endif
	#if MAX6682_ALLOW_UNITS == 1
		enum class TemperatureUnit{
			Celsius 	= 0x00,
			Kelvin		= 0x01,
			Fahrenheit 	= 0x02
		};
	#endif
	class MAX6682 {
		public:
			#if MAX6682_USE_SPIHANDLER == 1
				MAX6682(Handler Bus, GPIO_TypeDef *Port, uint16_t Pin);
			#else
				MAX6682(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin);
				MAX6682(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin, float Offset);
			#endif
			#if MAX6682_ALLOW_BUFFERS == 1
				float ReadBuffer(void);
			#endif
			float Read(void);
			#if MAX6682_ALLOW_UNITS == 1
				void SetUnit(TemperatureUnit Unit);
			#endif
			//Legacy Functions
			float ReadCelsius(void);
			#if MAX6682_ALLOW_UNITS == 1
				float ReadFahrenheit(void);
				float ReadFarenheit(void) { return ReadFahrenheit(); }
			#endif
			void Deselect(void);
			#if MAX6682_ALLOW_BUFFERS == 1
				void ZeroBuffer(void);
			#endif
			#if MAX6682_USE_WATCH_DOG ==1
                void LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog);
            #endif
		private:
			float Offset = 0.00;
			float LastMeasurement = 0;
			uint32_t LastCheck = 0;
			#if MAX6682_ALLOW_UNITS == 1
				TemperatureUnit Unit = TemperatureUnit::Celsius;
			#endif
			#if MAX6682_ALLOW_BUFFERS == 1
				#if MAX6682_ALLOCATE_BUFFERS == 1
					float *Buffer;
				#else
					float Buffer[MAX6682_BUFFER_LENGTH];
				#endif
			#endif
			//float Buffer[BUFFER_LENGTH];
			GPIO_TypeDef * Port;
			uint16_t Pin;
			#if MAX6682_USE_SPIHANDLER == 1
				Handler Bus;
			#else
				SPI_HandleTypeDef *Handle;
			#endif
			#if MAX6682_USE_WATCH_DOG ==1
				IWDG_HandleTypeDef   *hnl_watchdog = NULL;
				void RefreshWatchDog();
			#endif
			static void ConvertInteger(int16_t, float *, float * );
			//uint8_t SpiRead(void);
	};
#endif
