#ifndef __CH341A_H
#define __CH341A_H

#include "config.h"
#include "stm32f10x_conf.h"
#include "hw_config.h"
#include "usbd.h"

#define CH341A_BUF_SIZE             (CH341A_DATA_SIZE * CH341A_MAX_PACKETS)

#define CH341A_SIZ_DEVICE_DESC      18
#define CH341A_SIZ_CONFIG_DESC      39

#define ch341a_get_configuration       NOP_Process
#define ch341a_set_configuration       NOP_Process
#define ch341a_get_interface           NOP_Process
#define ch341a_set_interface           NOP_Process
#define ch341a_get_status              NOP_Process
#define ch341a_clear_feature           NOP_Process
#define ch341a_set_end_point_feature   NOP_Process
#define ch341a_set_device_feature      NOP_Process
#define ch341a_set_device_address      NOP_Process

#define CH341A_REQ_READ_VERSION     0x5F
#define CH341A_REQ_WRITE_REG        0x9A
#define CH341A_REQ_READ_REG         0x95
#define CH341A_REQ_SERIAL_INIT      0xA1
#define CH341A_REQ_MODEM_CTRL       0xA4

#define CH341A_CMD_SPI_STREAM       0xA8
#define CH341A_CMD_I2C_STREAM       0xAA
#define CH341A_CMD_UIO_STREAM       0xAB

#define CH341A_CMD_I2C_STM_SET      0x60 /* bit 2: SPI with two data pairs D5,D4=out, D7,D6=in */
#define CH341A_CMD_I2C_STM_END      0x00

#define CH341A_CMD_UIO_STM_DIR      0x40
#define CH341A_CMD_UIO_STM_OUT      0x80
#define CH341A_CMD_UIO_STM_END      0x20

#define CH341A_CMD_I2C_STM_SPEED_MASK   0x03
#define CH341A_CMD_I2C_STM_SPEED_20KHZ  0x00
#define CH341A_CMD_I2C_STM_SPEED_100KHZ 0x01
#define CH341A_CMD_I2C_STM_SPEED_400KHZ 0x02
#define CH341A_CMD_I2C_STM_SPEED_750KHZ 0x03
#define CH341A_CMD_I2C_STM_WIDTH_MASK   0x04
#define CH341A_CMD_I2C_STM_WIDTH_SINGLE 0x00
#define CH341A_CMD_I2C_STM_WIDTH_DOUBLE 0x04

#define CH341A_CLKRATE              48000000

#define CH341A_REGS_COUNT           0x27

#define CH341A_REG_BREAK            0x05
#define CH341A_REG_STATUS1          0x06
#define CH341A_REG_STATUS2          0x07
#define CH341A_REG_PRESCALER        0x12
#define CH341A_REG_DIVISOR          0x13
#define CH341A_REG_LCR              0x18
#define CH341A_REG_LCR2             0x25

#define CH341A_NBREAK_BITS          0x01

#define CH341A_LCR_ENABLE_RX        0x80
#define CH341A_LCR_ENABLE_TX        0x40
#define CH341A_LCR_MARK_SPACE       0x20
#define CH341A_LCR_PAR_EVEN         0x10
#define CH341A_LCR_ENABLE_PAR       0x08
#define CH341A_LCR_STOP_BITS_2      0x04
#define CH341A_LCR_CS_MASK          0x03
#define CH341A_LCR_CS8              0x03
#define CH341A_LCR_CS7              0x02
#define CH341A_LCR_CS6              0x01
#define CH341A_LCR_CS5              0x00

#define CH341A_BIT_CTS              0x01
#define CH341A_BIT_DSR              0x02
#define CH341A_BIT_RI               0x04
#define CH341A_BIT_DCD              0x08
#define CH341A_BIT_DTR              0x20
#define CH341A_BIT_RTS              0x40


typedef enum {
    Mode_None = 0, Mode_SPI, Mode_UART,
    Modes_Count
} Ch341A_Mode;

extern Ch341A_Mode ch341a_mode;
extern uint32_t ch341a_spi_fixed_speed;
extern uint32_t ch341a_spi_speed_factor;
extern uint32_t ch341a_tx_total;
extern uint32_t ch341a_rx_total;
extern const char *ch341a_mode_names[];

extern volatile int usb_tx_in_progress;
extern volatile int usb_rx_in_progress;
extern volatile int spi_tx_in_progress;


void ch341a_on_ep1_in(void);
void ch341a_on_packet_transmitted(void);
void ch341a_on_packet_received(const uint8_t *buf, uint8_t length);
void ch341a_on_frame_interval(void);

void ch341a_serial_irq(void);
void ch341a_spi_irq(void);

#endif /* __CH341A_H */
