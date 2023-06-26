#include "Driver.h"
#include "Clockings.h"

/**
 * Initialization routine.
 * You might need to enable access to DWT registers on Cortex-M7
 *   DWT->LAR = 0xC5ACCE55
 */
#if USE_CLOCKINGS == 1
	#if defined(DWT_BASE) && !defined(DWT_DELAY_DISABLED)
		void DWT_Init(void){
			if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
				CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
				DWT->CYCCNT = 0;
				DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
			}
		}
	#endif
	void DelayMicroSeconds(uint32_t us){
		#if defined(DWT_BASE) && !defined(DWT_DELAY_DISABLED)
			uint32_t startTick = DWT->CYCCNT, delayTicks = us * (SystemCoreClock/1000000);
			while (DWT->CYCCNT - startTick < delayTicks);
		#else
			__IO uint32_t currentTicks = SysTick->VAL;
			const uint32_t tickPerMs = SysTick->LOAD + 1;
			const uint32_t nbTicks = ((us - ((us > 0) ? 1 : 0)) * tickPerMs) / 1000;
			uint32_t elapsedTicks = 0;
			__IO uint32_t oldTicks = currentTicks;
			do {
				currentTicks = SysTick->VAL;
				elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks : oldTicks - currentTicks;
				oldTicks = currentTicks;
			} while (nbTicks > elapsedTicks);
		#endif
	}
	uint32_t micros(void){
		#if defined(DWT_BASE) && !defined(DWT_DELAY_DISABLED)
			return  DWT->CYCCNT / (SystemCoreClock / 1000000U);
		#else
			__IO uint32_t currentTicks = SysTick->VAL;
			//const uint32_t tickPerMs = SysTick->LOAD + 1;
			return currentTicks;
		#endif
	}

#endif
