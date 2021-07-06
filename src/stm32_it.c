#include "stm32_it.h"
#include "console.h"
#include "usbd.h"
#include "ch341a.h"

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
    while (1) {
    }
}

void MemManage_Handler(void) {
    while (1) {
    }
}

void BusFault_Handler(void) {
    while (1) {
    }
}

void UsageFault_Handler(void) {
    while (1) {
    }
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
}

void USB_IRQ_HANDLER(void) {
    usbd_istr();
}

void CONSOLE_IRQ_HANDLER(void) {
    console_irq();
}

void SERIAL1_IRQ_HANDLER(void) {
    ch341a_serial_irq();
}

void SPI_IRQ_HANDLER(void) {
    ch341a_spi_irq();
}
