#include "max6682.h"
#include <stdlib.h>
#include "Clockings.h"
//GPIO_#TypeDef *GPIOx, uint16_t
#if MAX6682_USE_SPIHANDLER == 1
	MAX6682::MAX6682(Handler Bus, GPIO_TypeDef *Port, uint16_t Pin) {
		this->Port = Port;
		this->Pin = Pin;
		this->Bus = Bus;
		#if MAX6682_ALLOW_BUFFERS == 1
			#if MAX6682_ALLOCATE_BUFFERS == 1
				this->Buffer = (float *)malloc(MAX6682_BUFFER_LENGTH*sizeof(float));
			#endif
		#endif
		Deselect();
		#if MAX6682_ALLOW_BUFFERS == 1
			ZeroBuffer();
		#endif
	}
#else
	MAX6682::MAX6682(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin){
		this->Port = Port;
		this->Pin = Pin;
		this->Handle = Handle;
		#if MAX6682_ALLOW_BUFFERS == 1
			#if MAX6682_ALLOCATE_BUFFERS == 1
				this->Buffer = (float *)malloc(MAX6682_BUFFER_LENGTH*sizeof(float));
			#endif
		#endif
		Deselect();
		#if MAX6682_ALLOW_BUFFERS == 1
			ZeroBuffer();
		#endif
	}
	MAX6682::MAX6682(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin, float Offset){
		this->Port = Port;
		this->Pin = Pin;
		this->Handle = Handle;
		#if MAX6682_ALLOW_BUFFERS == 1
			#if MAX6682_ALLOCATE_BUFFERS == 1
				this->Buffer = (float *)malloc(MAX6682_BUFFER_LENGTH*sizeof(float));
			#endif
		#endif
		this->Offset = Offset;
		Deselect();
		#if MAX6682_ALLOW_BUFFERS == 1
			ZeroBuffer();
		#endif
	}
#endif
#if MAX6682_ALLOW_BUFFERS == 1
	void MAX6682::ZeroBuffer(){
		for(int i=0;i<MAX6682_BUFFER_LENGTH;i++){
			#if MAX6682_ALLOCATE_BUFFERS == 1
				*(this->Buffer + i) = 0.00;
			#else
				Buffer[i] = 0.00f;
			#endif
		}
	}
#endif
void MAX6682::Deselect(){
	if (Port != NULL){
		HAL_GPIO_WritePin(Port, Pin, GPIO_PIN_SET);
	}
}
#if MAX6682_ALLOW_UNITS == 1
	void MAX6682::SetUnit(TemperatureUnit Unit){
		this->Unit = Unit;
	}
#endif
#if MAX6682_ALLOW_BUFFERS == 1
	float MAX6682::ReadBuffer(){
		float Result = 0;
		for(int i=0;i<MAX6682_BUFFER_LENGTH-1;i++){
			#if MAX6682_ALLOCATE_BUFFERS == 1
				*(this->Buffer+i) = *(this->Buffer+i+1);
				Result += *(this->Buffer+i);
			#else
				this->Buffer[i] = this->Buffer[i+1];
				Result += this->Buffer[i];
			#endif
		}
		#if MAX6682_ALLOCATE_BUFFERS == 1
			*(this->Buffer+MAX6682_BUFFER_LENGTH-1) = Read();
			Result += *(this->Buffer+MAX6682_BUFFER_LENGTH-1);
		#else
			Buffer[MAX6682_BUFFER_LENGTH-1] = Read();
			Result += this->Buffer[MAX6682_BUFFER_LENGTH-1];
		#endif
		return Result / (float)MAX6682_BUFFER_LENGTH;
	}
#endif
#if MAX6682_ALLOW_UNITS == 1
	float MAX6682::ReadCelsius(void) {
		//uint16_t v;
		//SPIHandler::BeginTransaction(this->Bus, this->Port, *this->Pin);
		//v = (char)(SPIHandler::ReadData(this->Bus,1000));
		//v <<= 8;
		//v |= (char)(SPIHandler::ReadData(this->Bus,1000));
		//SPIHandler::EndTransaction(this->Bus);
		//if (v & 0x4) {
		//  return 0;//NAN;
		//}
		//v >>= 3;
		//return v * 0.25;
		return Read();
	}
	float MAX6682::ReadFahrenheit(void) { return ReadCelsius() * 9.0 / 5.0 + 32; }
#endif
float MAX6682::Read(){
	int16_t v;
	//if ((HAL_GetTick() - LastCheck) >= 3){//300
		LastCheck = HAL_GetTick();
		#if MAX6682_USE_WATCH_DOG ==1
			RefreshWatchDog();
		#endif
		#if MAX6682_USE_SPIHANDLER == 1
			uint8_t a = 0;
			uint8_t b = 0;
			SPIHandler::BeginTransaction(this->Bus, this->Port, this->Pin);
			a = (SPIHandler::ReadData(this->Bus,1000));
			b = (SPIHandler::ReadData(this->Bus,1000));
			SPIHandler::EndTransaction(this->Bus);
			v = (int16_t)(a << 3)|(int16_t)(b>>3);
		#else
			uint8_t rx_buffer = 0;
			HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_RESET);
			HAL_SPI_Receive(Handle, &rx_buffer, sizeof(uint8_t), 100);
			v = (int16_t)(rx_buffer << 3);
			HAL_SPI_Receive(Handle, &rx_buffer, sizeof(uint8_t), 100);
			v |= (int16_t)(rx_buffer >> 5);
			HAL_GPIO_WritePin(this->Port, this->Pin, GPIO_PIN_SET);
		#endif
		#if MAX6682_USE_WATCH_DOG ==1
			RefreshWatchDog();
		#endif
		float output = 0;
		//SDDDDDDD DDDDXXXX
		ConvertInteger(v,&Offset,&output);
		//output = ((float)v * 0.125) + Offset;
		
		#if MAX6682_ALLOW_UNITS ==1
		if (Unit == TemperatureUnit::Kelvin){output = output + 273.15;}
		else if (Unit == TemperatureUnit::Fahrenheit){ output =  output  * 9.0 / 5.0 + 32;}
		#endif
		LastMeasurement = output;
		return output;
	//}
	//else{
	//	return LastMeasurement;
	//}
}
void MAX6682::ConvertInteger(int16_t ReadValue, float * InputOffset, float * OutputValue){
	*OutputValue = ((float)ReadValue * 0.125) + *InputOffset;
}
//uint8_t MAX6682::SpiRead(void) {
//	//int i;
//	char d = 0;
//	SPIHandler::BeginTransaction(this->Bus, this->Port, *this->Pin);
//	d = (char)(SPIHandler::ReadData(this->Bus,1000));
//	SPIHandler::EndTransaction(this->Bus);
//	return d;
//}
#if MAX6682_USE_WATCH_DOG ==1
	void MAX6682::RefreshWatchDog(){
		if (hnl_watchdog == NULL){ return; }
		HAL_IWDG_Refresh(hnl_watchdog);
	}
	void MAX6682::LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog){
		hnl_watchdog = HandleWatchDog;
		RefreshWatchDog();
	}
#endif
