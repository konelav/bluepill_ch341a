#ifndef __TIMER_H
#define __TIMER_H

#include "hw_config.h"

/* TIM2 & TIM3 are used in cascade (maser-slave) mode to make 32-bit resolution */

void timer_init(void);
uint32_t timer_usec(void);
void timer_wait_for_usec(uint32_t t);
void timer_delay_usec(uint32_t dt);

#endif /* __TIMER_H */
