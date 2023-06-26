#ifndef __GENERICS_H
	#define __GENERICS_H




	#ifdef __cplusplus
		#include "Driver.h"
		#include "WString.h"

		extern "C" {
	#endif
			#include "Clockings.h"
	//if defined (  __GNUC__  ) /* GCC CS3 */
	//	#define WEAK __attribute__ ((weak))
	//#endif

	//uint32_t millis() {
	//  return HAL_GetTick();
	//}
	#ifdef __cplusplus
		}	
	#endif

#endif
