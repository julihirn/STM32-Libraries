#ifndef __COMMS_H
    #define __COMMS_H
	#include "Communications.h"
	#include "Actor.h"

	#ifdef __cplusplus
	    extern "C" {
	#endif

	#ifdef __cplusplus
	}
	#endif
	#include "W25X40CL.h"

	#define COMMAND_MARK_IN 	0x43
	#define COMMAND_MARK_INIT 	0x3A
	#define COMMAND_MARK_OUT	0x3B
	#define COMMAND_EQUALITY	0x3D

	

    enum CommsState{
    	COMMS_OK = 0x00,
    	COMMS_ERROR = 0x01,
		COMMS_PROCESSED = 0x02
    };
	enum CommsReceiveState{
        RECV_READY = 0x00,
		RECV_ARMMED = 0x01,
		RECV_INCOMMAND = 0x02,
		RECV_COMPLETE = 0x03,
		RECV_TIMEOUT = 0x04
    };
	enum CommsDuplex{
		DUPLEX_HALF = 0x00,
		DUPLEX_FULL = 0x01
	};
	class Command{
		public:
			Command();
			uint8_t *Data;
			size_t Length;
	};
	struct ActorWrapper{
		Actor * Link;
	};
	#if USE_ACTORS == 0
		

	#endif
	class CommandHandler{
		public:
			static void Initalise(UART_HandleTypeDef *, uint8_t *);
			static void Initalise(UART_HandleTypeDef *, uint8_t *, unsigned long *);
			static void Initalise(UART_HandleTypeDef *, uint8_t *, unsigned long *, GPIO_TypeDef *, int16_t);
			
			static void SetDuplex(CommsDuplex duplex);
			//static void Initalise(UART_HandleTypeDef *, uint8_t *);
			#if USE_ACTORS == 1
				#if USE_VECTORS == 1
					static std::vector<Actor *> Actors;
				#else
					static ActorWrapper * Actors;
					static uint8_t Count;
				#endif
			#endif
			static CommsState ProcessCommand(uint8_t * Input);
			#if USE_ACTORS == 1
				static void AddActor(Actor * NewActor);
				static CommsState PublishStates(ActorDirection States);
				static CommsState PublishActor(Actor * Input);
			#endif
			#if USE_ACTORS == 1
				static void ParseCommand();
				static uint8_t FindActor(String & Name);
				static void InvalidateAll();

				static void EnableCategory(uint8_t Category, bool State);
			#else
				static Command ParseCommand();
			#endif
			static void CheckTimeOuts();
			static void TransmitString(String &);
			static void TransmitString(char *);
			static void TransmitErrorStatus(bool IsError, uint8_t ErrorCode);

			static void AppendNumber(String & Input, int Value, uint8_t Padding);
			#if USE_WATCH_DOG ==1
				static void LinkWatchDog(IWDG_HandleTypeDef * HandleWatchDog);
			#endif
			#if USE_MEMORY_CONTROLLER == 1
				static void SetupMemoryController(W25X40CL *, uint32_t);
			#endif
			static bool LineBusy;
		private:
			static bool CheckAddress(String &);
			#if USE_ACTORS == 1
				static bool AssignActor(String &);
			#endif
			static void Transmit(String &);
			static void EnableTransmit(bool);
			
			#if USE_WATCH_DOG == 1
				static void RefreshWatchDog();
			#endif
			
			static unsigned long * UID;

			#if USE_WATCH_DOG == 1
				static IWDG_HandleTypeDef * hnl_watchdog;
			#endif

			//Buffers
			static uint8_t * CommsBuffer;
			static uint8_t *buffer;
			static volatile Command * command;
			static unsigned char currentByte;
			static bool FirstRun;

			//Event Ticks
			static uint32_t tickstart;
			static uint32_t tickHdnl;
			static uint32_t LastReceived;

			//Handles
			static UART_HandleTypeDef *handle;
			static GPIO_TypeDef * TXEnablePort;
			static int16_t TXEnablePin;

			//System State
			static bool Initalised;
			static bool UnprocessedCommand;
			
			static bool HasTransmitEnable;
			static CommsReceiveState ReceiveState;
			static CommsDuplex Duplex;
			
			//Memory Controller
			#if USE_MEMORY_CONTROLLER == 1
				static W25X40CL *Memory;
				static uint32_t StartAddress;
				static uint8_t BufferObjects;
			#endif
	};

#endif
