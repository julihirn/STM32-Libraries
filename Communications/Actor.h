/*
	ACTORS
	
	Description:
	----------------------------------
	The Actors provide a means to interface the input or output of a sensor 
	or a parameter. The Actor stores the state, the type and the direction 
	of the device. 
	
	Using this library:
	----------------------------------
	1. Enable the use of Actors from "Communications.h", and set USE_ACTORS to 1
	2. Declare an Actor: Actor(<NAME>,<DIRECTION>,<PORT_TYPE>);
			Example: Actor Sensor = new Actor("SENSOR", ActorDirection::Output, ActorDataType::Analog)
	3. Reference this Actor for quick direct access.
			Example: &Sensor


*/
#ifndef _ACTOR_H
    #define _ACTOR_H

	#include "Communications.h"
	#if ACTOR_ALLOW_TIME == 1
		#include "ActorTime.h"
	#endif

	#define FLAG_DEFAULT_OFF			0x00
	#define FLAG_ISMODIFIED				0x01
	#define FLAG_CANTRIGGERFUNCTION		0x02
    #if USE_ACTORS == 1
        enum class ActorDirection{
        	Output = 0x00,
        	Input = 0x01,
			NoDirection = 0x02
        };
        enum class ActorDataType{
        	Float = 0x00,
			Bool = 0x01,
			Int8 = 0x02,
			Int16 = 0x03,
			Int32 = 0x04,
			Time = 0x05,
			Void = 0x06,
			PassThrough = 0xFF
        };
        constexpr enum ActorDirection operator |( const enum ActorDirection selfValue, const enum ActorDirection inValue ){
            return (enum ActorDirection)(uint8_t(selfValue) | uint8_t(inValue));
        }
        constexpr enum ActorDirection operator &( const enum ActorDirection selfValue, const enum ActorDirection inValue ){
            return (enum ActorDirection)(uint8_t(selfValue) & uint8_t(inValue));
        }
        enum ActorState{
        	ACTOR_OK = 0x00,
        	ACTOR_TYPE_ERROR = 0x01,
        	ACTOR_BUILD_ERROR = 0X02,
        	ACTOR_OUT_CHANGED = 0x04,
        	ACTOR_IN_CHANGED = 0x08,
        	ACTOR_VAILD = 0x10,
        	ACTOR_NOT_HANDLED = 0x20,
			ACTOR_OUT_OF_RANGE = 0x40,
			ACTOR_MISMATCH	= 0x80
        };
        class Actor{
	        public:
	        	Actor();
	        	Actor(String Name, ActorDirection Direction, ActorDataType Type);
				Actor(String Name, uint8_t ID, ActorDirection Direction, ActorDataType Type);
				Actor(String Name, ActorDirection Direction, ActorDataType Type, void (*OnChange)(Actor *));
				Actor(String Name, uint8_t ID, ActorDirection Direction, ActorDataType Type, void (*OnChange)(Actor *));
				
	        	~Actor();
    
	        	//void Redefine(String Name, ActorDirection Direction, ActorDataType Type);
    
				ActorState SetData(String &);
				ActorState SetData(String &, bool);

				ActorState GetData(String *);
				
				ActorState GetData(bool *);
				ActorState SetData(bool);
	        	ActorState SetData(bool, bool);
				//8 bit types
				#if ACTOR_ALLOW_INT8 == 1
					//8-BIT Getters
					ActorState GetData(char *);
					ActorState GetData(uint8_t *);
					ActorState GetData(int8_t *);

					//b-Bit Setters
					ActorState SetData(char);
					ActorState SetData(uint8_t);
					ActorState SetData(int8_t);

					ActorState SetData(char, bool);
					ActorState SetData(uint8_t, bool);
					ActorState SetData(int8_t, bool);
				#endif
				//16 bit types
				#if ACTOR_ALLOW_INT16 == 1
					//16-BIT Getters
					ActorState GetData(int *);
					ActorState GetData(uint16_t *);
					ActorState GetData(int16_t *);

					ActorState SetData(int);
					ActorState SetData(uint16_t);
					ActorState SetData(int16_t);

					ActorState SetData(int, bool);
					ActorState SetData(uint16_t, bool);
					ActorState SetData(int16_t, bool);
				#endif
				//32 bit type
				#if ACTOR_ALLOW_INT32 == 1
					//32-BIT Getters
					ActorState GetData(long *);
					ActorState GetData(uint32_t *);
					ActorState GetData(int32_t *);

					ActorState SetData(long);
					ActorState SetData(uint32_t);
					ActorState SetData(int32_t);

					ActorState SetData(long, bool);
					ActorState SetData(uint32_t, bool);
					ActorState SetData(int32_t, bool);
				#endif
				//Floats
				#if ACTOR_ALLOW_FLOATS == 1
	        		ActorState GetData(float *);

					ActorState SetData(float);
					ActorState SetData(float, bool);
				#endif
				#if ACTOR_ALLOW_TIME == 1
					ActorState GetData(TimeStruct *);

					ActorState SetData(TimeStruct *, bool);
					ActorState SetData(TimeStruct *);
				#endif

	        	bool GetState(void);
				void ResetState(void);
	        	ActorDirection GetDirection();
				ActorDataType GetType();
	        	
				#if USE_ADDRESSING == 1
	        		ActorState BuildCommandString(String * Command, unsigned long *id);
				#else
					ActorState BuildCommandString(String * Command);
				#endif
				uint8_t UniqueID = 0;
				uint8_t Category = 0;
				String * GetName();

				void Invalidate(void);

				void Enable(bool Set);
				bool IsEnabled();
	        private:
	        	String Command = "";
				void (*OnChangeEvent)(Actor *) = 0;

				void TriggerOnChangeEvent(bool Trigger);
				
				uint8_t	Flags = FLAG_DEFAULT_OFF;

	        	ActorDirection Direction = ActorDirection::Output;
	        	ActorDataType Type = ActorDataType::Bool;

				bool Enabled = true;

				void * Data;

				#if ACTOR_ALLOW_INT8 == 1
					static ActorState GetBaseInt8(Actor * Input, int8_t * Data);
				#endif
				#if ACTOR_ALLOW_INT16 == 1
					static ActorState GetBaseInt16(Actor * Input, int16_t * Data);
				#endif
				#if ACTOR_ALLOW_INT32 == 1
					static ActorState GetBaseInt32(Actor * Input, int32_t * Data);
				#endif

				#if ACTOR_ALLOW_INT8 == 1
					static ActorState SetBaseData(Actor * Input, int8_t Data);
				#endif
				#if ACTOR_ALLOW_INT16 == 1
					static ActorState SetBaseData(Actor * Input, int16_t Data);
				#endif
				#if ACTOR_ALLOW_INT32 == 1
					static ActorState SetBaseData(Actor * Input, int32_t Data);
				#endif
				#if ACTOR_ALLOW_FLOATS == 1
					static ActorState SetBaseData(Actor * Input, float Data);
				#endif

				static void AllocateMemory(Actor *, ActorDataType);
				static void FormatInteger(String * Str, uint8_t Input, uint8_t LeadingZeros);
	        	//bool * DigitalData;
				//#if ACTOR_ALLOW_ANALOG == 1
	        	//	float AnalogData = 0;
				//#endif
				//#if ACTOR_ALLOW_ENUMS == 1
				//	int EnumData = 0;
				//#endif
				//#if ACTOR_ALLOW_TIME == 1
				//	TimeStruct * Time;
				//#endif
//
				//uint32_t LongData = 0;
				//uint8_t CharData = 0;
        };
		namespace ActorSupport{
    		bool GetBoolean(Actor * Input);

    		uint8_t GetUint8(Actor * Input);
    		int8_t GetInt8(Actor * Input);
    		char GetChar(Actor * Input);

    		uint16_t GetUint16(Actor * Input);
    		int16_t GetInt16(Actor * Input);
    		int GetInt(Actor * Input);
			float GetFloat(Actor * Input);
		};
	#endif
#endif
