#include "timer.h"

void timer_init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    {
        TIM_TimeBaseInitTypeDef s;
        s.TIM_Period = 0xffff;
        s.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
        s.TIM_ClockDivision = 0;
        s.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM2, &s);
        s.TIM_Prescaler = 0;
        TIM_TimeBaseInit(TIM3, &s);
    }

    TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);
    TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_External1);
    TIM_SelectInputTrigger(TIM3, TIM_TS_ITR1);

    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

uint32_t timer_usec(void) {
    uint32_t lo = TIM2->CNT,
             hi = TIM3->CNT;
    return (hi << 16) | lo;
}

void timer_wait_for_usec(uint32_t t) {
    while ((int32_t)(t - timer_usec()) > 0)
        ;
}

void timer_delay_usec(uint32_t dt) {
    timer_wait_for_usec(timer_usec() + dt);
}
