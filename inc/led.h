#ifndef __LED_H
#define __LED_H

#include "config.h"
#include "hw_config.h"

void led_init(void);
int led_is_on(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

void led_set_period(uint32_t period_usec);
void led_blink(void);

#endif /* __LED_H */
