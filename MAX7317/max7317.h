#ifndef _MAX7317_H
	#define _MAX7317_H
    #include "Driver.h"
	#include "Generics.h"
    enum class MAX7317Command {
        Port0Level      = 0x00,		// Port P0 output level
        Port1Level      = 0x01,		// Port P1 output level
        Port2Level      = 0x02,		// Port P2 output level
        Port3Level      = 0x03,		// Port P3 output level
        Port4Level      = 0x04,		// Port P4 output level
        Port5Level      = 0x05,		// Port P5 output level
        Port6Level      = 0x06,	    // Port P6 output level
        Port7Level      = 0x07,		// Port P7 output level
        Port8Level      = 0x08,		// Port P8 output level
        Port9Level      = 0x09,		// Port P9 output level
        WriteAllPorts   = 0x0A,		// Write ports P0 through P9 with same output level
        WritePorts0_3   = 0x0B,		// Write ports P0 through P3 with same output level
        WritePorts4_7   = 0x0C,		// Write ports P4 through P7 with same output level
        WritePorts8_9   = 0x0D,		// Write ports P8 or P9 with same output level
        ReadPorts0_7    = 0x8E,		// Read ports P7 through P0 inputs
        ReadPorts8_9    = 0x8F		// Read ports P9 and P8 inputs
    };// __attribute__((packed));
	class MAX7317 {
		public:
			#if MAX7317_USE_SPIHANDLER == 1
				MAX7317(Handler Bus, GPIO_TypeDef *Port, uint16_t Pin);
			#else
				MAX7317(SPI_HandleTypeDef *Handle, GPIO_TypeDef *Port, uint16_t Pin);
			#endif
			uint16_t Read(void);
            void InvertAll(bool);
            void Set(uint8_t, bool);
            void SetAll(bool State);
            void Push(uint16_t State);
            void PushBuffer();

            void Deselect();
		private:
            bool InvertSets = false;
            uint16_t Receive(MAX7317Command Command);
            void Write(MAX7317Command Command, uint8_t Data);
			GPIO_TypeDef * Port;
			uint16_t Pin;
			SPI_HandleTypeDef *Handle;

            uint16_t Buffer = 0x00;
	};
#endif
