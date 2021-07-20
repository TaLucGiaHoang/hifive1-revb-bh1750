/*
 * delay.c
 *
 *  Created on: July 21, 2021
 *      Author: Hoang Ta
 */
#include <stdint.h>
#include <metal/time.h>
#include <metal/timer.h>

// Return current time in milliseconds
unsigned long long millis(void) {
    int rv;
    unsigned long long mcc, timebase;
    rv = metal_timer_get_cyclecount(0, &mcc);  // get current clock
    if (rv != 0) {
        return -1;
    }
    rv = metal_timer_get_timebase_frequency(0, &timebase);
    if (rv != 0) {
        return -1;
    }
    return mcc * 1000 / timebase;
}

void delayMicroseconds(int microseconds)
{
	volatile uint32_t ul;
	for(ul = 0; ul < microseconds; ul++)
	{
		__asm__ volatile ("nop");
	}
}

void delay(uint32_t miliseconds)
{
	volatile uint32_t ul;
	for(ul = 0; ul < miliseconds; ul++)
	{
		delayMicroseconds(1000);
	}
}


