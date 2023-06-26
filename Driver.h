#ifndef DRIVER_H
#define DRIVER_H
	#include "stm32f412rx.h"
	#include "stm32f4xx_hal.h"

	#ifdef _WIN32
		#define uint32_t unsigned long
	#endif
	//SPI Handler Settings
	#define SPI_TRANSACTION_INITALISES	1
	#define SPI_TRANSACT_DELAY			0
	//MAX6675 Settings
	#define MAX6682_USE_SPIHANDLER		0
	#define MAX6682_ALLOW_BUFFERS		1
	#define MAX6682_ALLOCATE_BUFFERS	0
	#define MAX6682_ALLOW_UNITS			0
	#define MAX6682_BUFFER_LENGTH		5
	#define MAX6682_USE_WATCH_DOG		1

	#define PRINTABLE_ENABLE			0
	#define USE_DTOSTR					1

	#define W25X40CL_DEBUG_MESSAGES		0
	#define W25X40CL_USE_WATCH_DOG		1
#endif
