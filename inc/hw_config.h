#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "config.h"
#include "stm32f10x_conf.h"

/*
 * B10, B11             - USART3 (TX, RX)               - console       [5V FT]
 * A8, A9, A10, A11, A12- USART1 (CK, TX, RX, CTS, RTS) - CH341A.UART   [5V FT]
 * B12, B13, B14, B15   - SPI2 (NSS, SCK, MISO, MOSI)   - CH341A.SPI    [5V FT]
 * A11, A12             - USB (DM, DP)                  - USB FS device
 * C13                  - LED                           - led
 */

#define USB_GPIO                GPIOA
#define USB_DM_PIN              GPIO_Pin_11
#define USB_DP_PIN              GPIO_Pin_12
#define USB_IRQ                 USB_LP_CAN1_RX0_IRQn
#define USB_IRQ_HANDLER         USB_LP_CAN1_RX0_IRQHandler

#define CONSOLE_GPIO            GPIOB
#define CONSOLE_TX              GPIO_Pin_10
#define CONSOLE_RX              GPIO_Pin_11
#define CONSOLE_IRQ             USART3_IRQn
#define CONSOLE_IRQ_HANDLER     USART3_IRQHandler
#define CONSOLE_USART           USART3

#define SERIAL1_GPIO            GPIOA
#define SERIAL1_TX              GPIO_Pin_9
#define SERIAL1_RX              GPIO_Pin_10
#define SERIAL1_CTS             GPIO_Pin_11
#define SERIAL1_RTS             GPIO_Pin_12
#define SERIAL1_IRQ             USART1_IRQn
#define SERIAL1_IRQ_HANDLER     USART1_IRQHandler
#define SERIAL1_USART           USART1

#define SPI_GPIO                GPIOB
#define SPI_CS                  GPIO_Pin_12
#define SPI_CLK                 GPIO_Pin_13
#define SPI_MISO                GPIO_Pin_14
#define SPI_MOSI                GPIO_Pin_15
#define SPI_IRQ                 SPI2_IRQn
#define SPI_IRQ_HANDLER         SPI2_IRQHandler
#define SPI_SPI                 SPI2

#define LED_GPIO                GPIOC
#define LED_1                   GPIO_Pin_13

#define IRQ_PRIO_GROUP_CFG      NVIC_PriorityGroup_2
#define CH341A_IO_IRQ_PRIO      0
#define USB_IRQ_PRIO            1
#define CONSOLE_IRQ_PRIO        2

/*
 * Another periphery in use:
 *   - TIM2 and TIM3 in chained counter mode, used by timer.c.
 */

void init_peripherals(void);

#endif  /*__HW_CONFIG_H*/
