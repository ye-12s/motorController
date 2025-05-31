#ifndef __UTILS_H__
#define __UTILS_H__

#include "stm32g4xx_hal.h"

#define __CURRENT_TICKS HAL_GetTick()

// *INDENT-OFF*
#define TIMER_TRIGGER_PUSH(tick) \
    do {\
        static uint32_t __old_time = 0; \
        if( __CURRENT_TICKS - __old_time >= tick || __old_time == 0 ) \
        { \
            __old_time = __CURRENT_TICKS;

#define TIMER_TRIGGER_POP() \
        } \
    }while( 0 );



#define STATE_TRIGGER_PUSH(status, initStatus) \
	do {\
		static uint32_t __old_status = initStatus; \
		if( status != __old_status ) \
		{ \
			__old_status = (uint32_t)status;

#define STATE_TRIGGER_POP() \
	    } \
	}while( 0 )
// *INDENT-ON*






#endif