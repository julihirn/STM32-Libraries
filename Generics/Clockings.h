#include <stdint.h>
#include "Driver.h"
#if USE_CLOCKINGS == 1
	#ifndef INC_DWT_DELAY_H_
		#define INC_DWT_DELAY_H_
		//#define DWT_DELAY_NEWBIE 0
		#if defined(DWT_BASE) && !defined(DWT_DELAY_DISABLED)
			void DWT_Init(void);
		#endif
		void DelayMicroSeconds(uint32_t us);
		uint32_t micros(void);
	#endif


#endif /* INC_DWT_DELAY_DWT_DELAY_H_ */
