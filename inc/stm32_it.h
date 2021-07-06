#ifndef __STM32_IT_H
#define __STM32_IT_H

#include "hw_config.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void USB_IO_IRQ_HANDLER(void);
void USB_WAKEUP_IRQ_HANDLER(void);
void CONSOLE_IRQ_HANDLER(void);
void SERIAL1_IRQ_HANDLER(void);
void SPI_IRQ_HANDLER(void);

#endif /* __STM32_IT_H */
