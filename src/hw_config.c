#include "hw_config.h"

#define GPIO_IN_USE(gpio) (USB_GPIO == (gpio) || CONSOLE_GPIO == (gpio) \
    || SERIAL1_GPIO == (gpio) || SPI_GPIO == (gpio) || LED_GPIO == (gpio))

#define USART_IN_USE(usart) (CONSOLE_USART == (usart) \
    || SERIAL1_USART == (usart))

#define SPI_IN_USE(spi) (SPI_SPI == (spi))


static void init_rcc(void) {
    if (GPIO_IN_USE(GPIOA))
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    if (GPIO_IN_USE(GPIOB))
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    if (GPIO_IN_USE(GPIOC))
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    if (USART_IN_USE(USART1))
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    if (USART_IN_USE(USART2))
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    if (USART_IN_USE(USART3))
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    if (SPI_IN_USE(SPI1))
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    if (SPI_IN_USE(SPI2))
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

static void init_usb_pins(void) {
    GPIO_InitTypeDef s;
    GPIO_StructInit(&s);
    s.GPIO_Speed = GPIO_Speed_50MHz;
    s.GPIO_Pin = USB_DM_PIN | USB_DP_PIN;
    s.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USB_GPIO, &s);
}

static void init_console_pins(void) {
    GPIO_InitTypeDef s;
    GPIO_StructInit(&s);
    s.GPIO_Speed = GPIO_Speed_2MHz;
    s.GPIO_Pin = CONSOLE_TX;
    s.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(CONSOLE_GPIO, &s);
    s.GPIO_Pin = CONSOLE_RX;
    s.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(CONSOLE_GPIO, &s);
}

static void init_serial1_pins(void) {
    GPIO_InitTypeDef s;
    GPIO_StructInit(&s);
    s.GPIO_Speed = GPIO_Speed_2MHz;
    s.GPIO_Pin = SERIAL1_TX | SERIAL1_RTS;
    s.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SERIAL1_GPIO, &s);
    s.GPIO_Pin = SERIAL1_RX | SERIAL1_CTS;
    s.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SERIAL1_GPIO, &s);
}

static void init_spi_pins(void) {
    GPIO_InitTypeDef s;
    GPIO_StructInit(&s);
    s.GPIO_Speed = GPIO_Speed_10MHz;
    s.GPIO_Pin = SPI_CS;
    s.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_GPIO, &s);
    s.GPIO_Pin = SPI_CLK | SPI_MOSI;
    s.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SPI_GPIO, &s);
    s.GPIO_Pin = SPI_MISO;
    s.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SPI_GPIO, &s);
}
static void init_led_pins(void) {
    GPIO_InitTypeDef s;
    GPIO_StructInit(&s);
    s.GPIO_Speed = GPIO_Speed_2MHz;
    s.GPIO_Pin = LED_1;
    s.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_GPIO, &s);
}

void init_peripherals(void) {
    init_rcc();
    init_usb_pins();
    init_console_pins();
    init_serial1_pins();
    init_spi_pins();
    init_led_pins();
}
