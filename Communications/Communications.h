#ifndef COMMUNICATIONS_H_
#define COMMUNICATIONS_H_
	#include "Generics.h"
	#include "stdio.h"
	#include "../System/SystemErrors.h"

	#ifndef _COMMS_DEFS
		#define _COMMS_DEFS
		namespace Defines{
			const String ErrorHeader = "ERR";
			const String StatusHeader = "STAT";
			const String CommandHeader = "C:";
			const String CONST_TRUE = "true";
			const char Term = ';';
			const char CharTrue = '1';
			const char CharFalse = '0';
			const String EmptyString = "";
			const char Comma = ',';
			const char Equals = '=';
		}
		
	#endif
	/*
		Communications and Actor
		General Settings
		--------------------------------------------------------------
	*/
	
	//GENERAL SETTINGS						VALUE			  TYPE
	#define USE_ACTORS						1				//Boolean
	#define USE_INTERRUPTS					1				//Boolean
	#define USE_VECTORS						0				//Boolean
	#define USE_MEMORY_CONTROLLER			0				//Boolean

	//COMMAND PROCESSING SETTINGS
	#define MAX_COMMAND_LENGTH 				100				//Integer
	#define MAX_TIMEOUT						400				//Integer
	#define MAX_TRANSMITBUFFER				20				//Integer
	#define MAX_TRANSMITLENGTH				1000			//Integer
	#define MAX_TRANSMITTIMEOUT				5000			//Integer
	#define COMMS_LASTTIMEOUT				2				//Integer

	//ADDRESSING AND COMMAND TRANSMISSION FORMATTING
	#define USE_ADDRESSING					0				//Boolean
	#define USE_CHECKSUM					1				//Boolean
	
	//DEBUG SETTINGS
	#define COMMSHDLR_DEBUG_MESSAGES		0				//Boolean
	#define ACTOR_DEBUG_MESSAGES			0				//Boolean

	//ACTORS SETTINGS
	#define ACTOR_MARK_DIFFERENCE_ONLY		1				//Boolean
	#define ACTOR_ALLOW_INT8				1				//Boolean
	#define ACTOR_ALLOW_INT16				1				//Boolean
	#define ACTOR_ALLOW_INT32				0				//Boolean
	#define ACTOR_ALLOW_FLOATS				1				//Boolean
	#define ACTOR_ALLOW_TIME				1				//Boolean

	//MEMORY BUFFER SETTINGS
	#define COMMS_MEMORY_STEP				0x1000			//Integer

	//CHECK SUM SETTINGS
	#define CHECKSUM_TERM_OFFSET			10				//Byte
	#define CHECKSUM_TERM_MINOROFFSET		5				//Byte

	//WATCH DOG SETTINGS
	#define USE_WATCH_DOG					1				//Boolean

	#if USE_VECTORS == 1
		#include <vector>
	#endif

#endif /* COMMUNICATIONS_COMMUNICATIONS_H_ */
