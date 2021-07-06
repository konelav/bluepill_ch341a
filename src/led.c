#include "led.h"
#include "timer.h"

#if LED_INVERT == 1
#  define LED_BIT_ON Bit_RESET
#  define LED_BIT_OFF Bit_SET
#else
#  define LED_BIT_ON Bit_SET
#  define LED_BIT_OFF Bit_RESET
#endif

static uint32_t blink_period = 0;
static uint32_t next_toggle = 0;

void led_init(void) {
    led_off();
    led_set_period(0);
}

int led_is_on(void) {
    if (GPIO_ReadOutputDataBit(LED_GPIO, LED_1) == LED_BIT_ON)
        return 1;
    return 0;
}

void led_on(void) {
    GPIO_WriteBit(LED_GPIO, LED_1, LED_BIT_ON);
}

void led_off(void) {
    GPIO_WriteBit(LED_GPIO, LED_1, LED_BIT_OFF);
}

void led_toggle(void) {
    if (led_is_on())
        led_off();
    else
        led_on();
}

void led_set_period(uint32_t period_usec) {
    next_toggle += period_usec - blink_period;
    blink_period = period_usec;
}

void led_blink(void) {
    if (blink_period > 0) {
        uint32_t now = timer_usec();
        if ((int32_t)(now - next_toggle) >= 0) {
            led_toggle();
            next_toggle = now + blink_period;
        }
    }
}
