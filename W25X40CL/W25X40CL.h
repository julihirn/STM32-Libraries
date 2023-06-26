#ifndef _W25X40CL_H
    #define _W25X40CL_H

    #include "Driver.h"
    #include "Generics.h"
	#if W25X40CL_USE_SPIHANDLER == 1
		#include "SPIHandler.h"
	#endif
    #define W25X40CL_FLAG_WRITE_ENABLED 0x01
    #define W25X40CL_FLAG_ISBUSY        0x02
    #define W25X40CL_FLAG_ISCONFIG      0x04
    #define W25X40CL_WRITE_PROTECTED    0x08
    #define W25X40CL_FLAG_ISTRANSACTION 0x10

    #define SIZE_BITS                   1
    #define SIZE_BYTE                   1 * SIZE_BITS
    #define SIZE_WORD                   2 * SIZE_BITS
    #define SIZE_DWORD                  4 * SIZE_BITS
    #define SIZE_QWORD                  8 * SIZE_BITS

    //
    //      Addressing System
    //
    //      BBSXXX
    //
    //      B = Block (64kB) 0 -> FF
    //      S = Sector (4kB) 0 -> F
    //      X = Address (1B) 0 -> FFF
    //
    enum class W25X40CLCommand {
        WriteEnable         = 0x06,		// Write Enable.
        WriteEnableVolatile = 0x50,		// Write Enable for Volatile Status Register.
        WriteDisable        = 0x04,		// Write Disable.
        ReadStatusRegister  = 0x05,		// Read Status Register.
        WriteStatusRegister = 0x01,		// Write Status Register.
        Read                = 0x03,		// Read Data.
        ReadFast            = 0x0B,		// Fast Read.
        ReadFastDual        = 0xBB,	    // Fast Read Dual.
        PageProgram         = 0x02,		// Page Program.
        SectorErase         = 0x20,		// Sector Erase (4 kB).
        BlockErase32kB      = 0x52,		// Block Erase (32 kB).
        BlockErase64kB      = 0xD8,		// Block Erase (64kB).
        EraseAll            = 0x60,		// Chip Erase.
        PowerDown           = 0xB9,		// Power-down
        ReleasePowerDown    = 0xAB,		// Release Power-down/Device ID.
        ReadManufacterID    = 0x90,		// Read Manufacturer/Device ID.
        ReadManufacterIDDual= 0x92,		// Read Manufacturer/Device ID Dual I/O.
        ReadJID             = 0x9F, 	// Read JEDEC ID.
        ReadUID             = 0x4B		// Read Unique ID.
    };// __attribute__((packed));
    class W25X40CL{
        public:
            #if W25X40CL_USE_SPIHANDLER == 1
                W25X40CL(Handler, GPIO_TypeDef *, uint16_t);
                W25X40CL(Handler, GPIO_TypeDef *, uint16_t, GPIO_TypeDef *, uint16_t);
            #else
                W25X40CL(SPI_HandleTypeDef *, GPIO_TypeDef *, uint16_t);
                W25X40CL(SPI_HandleTypeDef *, GPIO_TypeDef *, uint16_t, GPIO_TypeDef *, uint16_t);
            #endif
            void UseTransactions(bool);

            void Deselect(void);

            void SendCommand(W25X40CLCommand Command);
            
            void ReadData(uint32_t Address, uint8_t*, uint16_t);
            void WriteData(uint32_t, uint8_t*, uint16_t);

            //Transaction Writing
            void StartWriteTransaction(uint32_t Address);
            void EndTransaction();

            uint32_t WriteTransaction(char);
            uint32_t WriteTransaction(int);
            uint32_t WriteTransaction(long);
            uint32_t WriteTransaction(bool);
            uint32_t WriteTransaction(float);
            uint32_t WriteTransaction(double);

            uint32_t WriteTransaction(uint8_t);
            uint32_t WriteTransaction(uint16_t);
            uint32_t WriteTransaction(uint32_t);

            uint32_t WriteTransaction(uint8_t *Data, uint16_t Length);
            

            //Once Off Writing
            uint32_t Write(uint32_t, String &);

            uint32_t Write(uint32_t, char);
            uint32_t Write(uint32_t, int);
            uint32_t Write(uint32_t, long);
            uint32_t Write(uint32_t, bool);

            uint32_t Write(uint32_t, float);
            uint32_t Write(uint32_t, double);
            
            uint32_t Write(uint32_t, uint8_t);
            uint32_t Write(uint32_t, uint16_t);
            uint32_t Write(uint32_t, uint32_t);

            uint32_t Write(uint32_t, uint8_t*, uint8_t);

            uint32_t Read(uint32_t, String*);

            uint32_t Read(uint32_t, bool*);
            uint32_t Read(uint32_t, char*);
            uint32_t Read(uint32_t, int *);
            uint32_t Read(uint32_t, long *);

            uint32_t Read(uint32_t, float *);
            uint32_t Read(uint32_t, double *);

            uint32_t Read(uint32_t, uint8_t *);
            uint32_t Read(uint32_t, uint16_t *);
            uint32_t Read(uint32_t, uint32_t *);
            uint32_t Read(uint32_t, uint8_t *, uint8_t);

            static uint32_t Append(uint8_t *, uint32_t, bool);
            static uint32_t Append(uint8_t *, uint32_t, char);
            static uint32_t Append(uint8_t *, uint32_t, int);
            static uint32_t Append(uint8_t *, uint32_t, long);

            static uint32_t Append(uint8_t *, uint32_t, float);
            static uint32_t Append(uint8_t *, uint32_t, double);

            static uint32_t Append(uint8_t *, uint32_t, uint8_t);
            static uint32_t Append(uint8_t *, uint32_t, uint16_t);
            static uint32_t Append(uint8_t *, uint32_t, uint32_t);
            
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, bool * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, char * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, int * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, long * Out);

            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint8_t * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint16_t * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, uint32_t * Out);

            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, float * Out);
            static uint32_t Extract(uint8_t * Source, uint32_t Length, uint32_t Start, double * Out);

            void EraseSector(uint32_t);
            void EraseBlock(uint32_t Address);
            void EraseAll(void);

            void WriteProtect(bool);

            bool IsReady(void);
            uint8_t ReadState(void);

            void ReadJEDECID(void);

            static uint32_t DeriveAddress(uint8_t Block, uint8_t Sector, uint16_t Address);
            #if W25X40CL_USE_WATCH_DOG ==1
                void LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog);
            #endif
        private:
            bool AllowTransactions = false;
            uint16_t DeviceFlags = 0x00;
            GPIO_TypeDef * Port;
			uint16_t Pin;

            GPIO_TypeDef * WPPort;
			uint16_t WPPin;
            void SendAddress(uint32_t);
			#if W25X40CL_USE_SPIHANDLER == 1
				Handler Bus;
			#else
				SPI_HandleTypeDef *SPIHandle;
			#endif
            #if W25X40CL_USE_WATCH_DOG ==1
				IWDG_HandleTypeDef   *hnl_watchdog = NULL;
				void RefreshWatchDog();
			#endif
    };
#endif
