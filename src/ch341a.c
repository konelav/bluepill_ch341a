#include <string.h>
#include "ch341a.h"
#include "console.h"

/* USB Standard Device Descriptor */
const uint8_t CH341A_DeviceDescriptor[] = {
    0x12,   /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    0x10,
    0x01,   /* bcdUSB = 1.10 */
    0xff,   /* bDeviceClass: Vendor Specific */
    0x00,   /* bDeviceSubClass */
    0x02,   /* bDeviceProtocol */
    CH341A_MAX_PACKET_SIZE,   /* bMaxPacketSize0 */
    (CH341A_VID &  0xFF),
    (CH341A_VID >>    8),   /* idVendor = CH341A_PID */
    (CH341A_PID &  0xFF),
    (CH341A_PID >>    8),   /* idProduct = CH341A_VID */
    0x04,
    0x03,   /* bcdDevice = 3.04 */
    0,              /* Index of string descriptor describing manufacturer */
    0,              /* Index of string descriptor describing product */
    0,              /* Index of string descriptor describing the device's serial number */
    0x01    /* bNumConfigurations */
};
ONE_DESCRIPTOR Device_Descriptor = {
    (uint8_t*)CH341A_DeviceDescriptor,
    CH341A_SIZ_DEVICE_DESC
};

const uint8_t CH341A_ConfigDescriptor[] = {
    /*Configuration Descriptor*/
    0x09,   /* bLength: Configuration Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
    CH341A_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
    0x00,
    0x01,   /* bNumInterfaces: 1 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0x80,   /* bmAttributes */
    0x30,   /* MaxPower x2 mA */
    /*Interface Descriptor*/
    0x09,   /* bLength: Interface Descriptor size */
    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
    /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x03,   /* bNumEndpoints: One endpoints used */
    0xff,   /* bInterfaceClass: Vendor Specific */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* bInterfaceProtocol */
    0x00,   /* iInterface: */
    /*Endpoint 1 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType */
    0x82,   /* bEndpointAddress: EP 2 IN */
    0x02,   /* bmAttributes: Bulk */
    CH341A_DATA_SIZE,      /* wMaxPacketSize: */
    0x00,
    0x0,   /* bInterval: */
    /*Endpoint 2 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x02,   /* bEndpointAddress: (OUT2) */
    0x02,   /* bmAttributes: Bulk */
    CH341A_DATA_SIZE,      /* wMaxPacketSize: */
    0x00,
    0x0,   /* bInterval: */
    /*Endpoint 3 Descriptor*/
    0x07,   /* bLength: Endpoint Descriptor size */
    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN1) */
    0x03,   /* bmAttributes: Interrut */
    CH341A_INT_SIZE,      /* wMaxPacketSize: */
    0x00,
    0x1   /* bInterval: */
};
ONE_DESCRIPTOR Config_Descriptor = {
    (uint8_t*)CH341A_ConfigDescriptor,
    CH341A_SIZ_CONFIG_DESC
};

DEVICE Device_Table = {
    EP_NUM,
    1
};

static void ch341a_init(void);
static void ch341a_reset(void);
static void ch341a_status_in(void);
static void ch341a_status_out(void);
static RESULT ch341a_data_setup(uint8_t RequestNo);
static RESULT ch341a_nodata_setup(uint8_t RequestNo);
static RESULT ch341a_get_interface_setting(uint8_t Interface, uint8_t AlternateSetting);
static uint8_t *ch341a_get_device_descriptor(uint16_t Length);
static uint8_t *ch341a_get_config_descriptor(uint16_t Length);
static uint8_t *ch341a_get_string_descriptor(uint16_t Length);
DEVICE_PROP Device_Property = {
    ch341a_init,
    ch341a_reset,
    ch341a_status_in,
    ch341a_status_out,
    ch341a_data_setup,
    ch341a_nodata_setup,
    ch341a_get_interface_setting,
    ch341a_get_device_descriptor,
    ch341a_get_config_descriptor,
    ch341a_get_string_descriptor,
    0,
    CH341A_MAX_PACKET_SIZE
};
USER_STANDARD_REQUESTS User_Standard_Requests = {
    ch341a_get_configuration,
    ch341a_set_configuration,
    ch341a_get_interface,
    ch341a_set_interface,
    ch341a_get_status,
    ch341a_clear_feature,
    ch341a_set_end_point_feature,
    ch341a_set_device_feature,
    ch341a_set_device_address
};


const uint8_t ch341a_version[2] = {0x27, 0x00};
uint8_t ch341a_reg[2] = {0x00, 0x00};
uint8_t ch341a_regs[CH341A_REGS_COUNT];
uint32_t ch341a_spi_fixed_speed = 0;
uint32_t ch341a_spi_speed_factor = 1;

const char *ch341a_mode_names[Modes_Count] = {
    "none", "spi", "uart"
};

Ch341A_Mode ch341a_mode = Mode_None;
uint32_t ch341a_tx_total = 0;
uint32_t ch341a_rx_total = 0;

volatile int usb_tx_in_progress = 0;
volatile int usb_rx_in_progress = 0;
volatile int spi_tx_in_progress = 0;

static uint32_t modem_control = ~0;
static uint32_t spi_enabled = 0;
static volatile uint8_t  tx_buf[CH341A_BUF_SIZE];
static volatile uint32_t tx_ptr_begin = 0, tx_ptr_end = 0;
static volatile uint8_t  rx_buf[CH341A_BUF_SIZE];
static volatile uint32_t rx_ptr_begin = 0, rx_ptr_end = 0;

static void serial_init(void) {
    USART_InitTypeDef s;
    uint32_t divisor = ch341a_regs[CH341A_REG_DIVISOR];
    uint32_t prescaler = ch341a_regs[CH341A_REG_PRESCALER];
    uint32_t lcr = ch341a_regs[CH341A_REG_LCR];
    uint32_t lcr2 = ch341a_regs[CH341A_REG_LCR2];
    
    (void)lcr2; /* unused */
    
    DBG_STR("serial_init()");
    
    USART_StructInit(&s);
    
    { /* baudrate */
        uint32_t div = 0x100 - divisor;
        uint32_t ps = prescaler & 0x03;
        uint32_t fact = (prescaler >> 2) & 0x01;
        s.USART_BaudRate = CH341A_CLKRATE / ((1 << (12 - 3 * ps - fact)) * div);
        
        DBG_VAL(" div = ", div, 10, "");
        DBG_VAL(" ps = ", ps, 10, "");
        DBG_VAL(" fact = ", fact, 10, "");
        DBG_VAL(" requested baudrate = ", s.USART_BaudRate, 10, "");
    }
    
    { /* wordlength */
        uint32_t databits = lcr & CH341A_LCR_CS_MASK;
        switch (databits) {
        case CH341A_LCR_CS8:
            s.USART_WordLength = USART_WordLength_8b;
            break;
        default:
        case CH341A_LCR_CS5:
        case CH341A_LCR_CS6:
        case CH341A_LCR_CS7:
            DBG_VAL("requested word size not supported: 0x", databits, 16, ", switch to CS8");
            s.USART_WordLength = USART_WordLength_8b;
            break;
        }
        DBG_VAL("  wordlength = 0x", s.USART_WordLength, 16, "");
    }
    
    { /* stopbits */
        s.USART_StopBits = (lcr & CH341A_LCR_STOP_BITS_2) ?
                                         USART_StopBits_2 :
                                         USART_StopBits_1 ;
        DBG_VAL("  stops = 0x", s.USART_StopBits, 16, "");
    }
    
    { /* parity */
        s.USART_Parity = USART_Parity_No;
        if (lcr & CH341A_LCR_ENABLE_PAR) {
            s.USART_Parity = (lcr & CH341A_LCR_PAR_EVEN) ?
                                       USART_Parity_Even :
                                       USART_Parity_Odd  ;
            s.USART_WordLength = USART_WordLength_9b;
        }
        DBG_VAL("  parity = 0x", s.USART_Parity, 16, "");
    }
    
    { /* mode */
        s.USART_Mode = 0;
        if (lcr & CH341A_LCR_ENABLE_RX)
            s.USART_Mode |= USART_Mode_Rx;
        if (lcr & CH341A_LCR_ENABLE_TX)
            s.USART_Mode |= USART_Mode_Tx;
        DBG_VAL("  mode = 0x", s.USART_Mode, 16, "");
        
        #if CH341A_ALWAYS_ENABLE_RXTX == 1
        s.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        #endif
    }
    
    s.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    
    USART_Init(SERIAL1_USART, &s);
    
    {
        uint32_t real_baudrate;
        RCC_ClocksTypeDef RCC_ClocksStatus;
        RCC_GetClocksFreq(&RCC_ClocksStatus);
        
        if (SERIAL1_USART == USART1)
            real_baudrate = RCC_ClocksStatus.PCLK2_Frequency / SERIAL1_USART->BRR;
        else
            real_baudrate = RCC_ClocksStatus.PCLK1_Frequency / SERIAL1_USART->BRR;
        
        DBG_VAL("  real_baudrate = ", real_baudrate, 10, "");
        INF() {
            console_putstr("uart settings: baudrate=");
            console_putnum(real_baudrate, 10, 0);
            console_putstr("; bits=");
            console_putstr(
                s.USART_WordLength == USART_WordLength_8b ? "8" :
                s.USART_WordLength == USART_WordLength_9b ? "9" : "?");
            console_putstr("; stops=");
            console_putstr(
                s.USART_StopBits == USART_StopBits_1 ? "1" :
                s.USART_StopBits == USART_StopBits_2 ? "2" : "?");
            console_putstr("; parity=");
            console_putstr(
                s.USART_Parity == USART_Parity_No   ? "no"   :
                s.USART_Parity == USART_Parity_Odd  ? "odd"  :
                s.USART_Parity == USART_Parity_Even ? "even" : "?");
            console_putstr("; mode=");
            if (s.USART_Mode & USART_Mode_Rx)
                console_putstr("rx");
            if (s.USART_Mode & USART_Mode_Tx)
                console_putstr("tx");
            console_putstr("\r\n");
        }
    }
    
    USART_ITConfig(SERIAL1_USART, USART_IT_RXNE, ENABLE);
    
    NVIC_PriorityGroupConfig(IRQ_PRIO_GROUP_CFG);
    {
        NVIC_InitTypeDef s;
        s.NVIC_IRQChannel = SERIAL1_IRQ;
        s.NVIC_IRQChannelPreemptionPriority = CH341A_IO_IRQ_PRIO;
        s.NVIC_IRQChannelSubPriority = 0;
        s.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&s);
    }
    
    USART_Cmd(SERIAL1_USART, ENABLE);
}

static void update_mode(void) {
    Ch341A_Mode new_mode;
    if (spi_enabled)
        new_mode = Mode_SPI;
    else if ((~modem_control) & (CH341A_BIT_DTR | CH341A_BIT_RTS))
        new_mode = Mode_UART;
    else
        new_mode = Mode_None;
    if (ch341a_mode == new_mode)
        return;
    INF() {
        console_putstr("mode changed: ");
        console_putstr(ch341a_mode_names[ch341a_mode]);
        console_putstr(" => ");
        console_putstr(ch341a_mode_names[new_mode]);
        console_putstr("\r\n");
    }
    ch341a_mode = new_mode;
}

static uint8_t *req_read_version(uint16_t length) {
    DBG_VAL("req_read_version(length = ", length, 10, ")");
    if (length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(ch341a_version);
        return NULL;
    }
    return (uint8_t*)ch341a_version;
}

static uint8_t *req_read_reg(uint16_t length) {
    DBG_VAL("req_read_reg(length = ", length, 10, ")");
    
    if (pInformation->USBwValues.bw.bb0 < sizeof(ch341a_regs))
        ch341a_reg[0] = ch341a_regs[pInformation->USBwValues.bw.bb0];
    if (pInformation->USBwValues.bw.bb1 < sizeof(ch341a_regs))
        ch341a_reg[1] = ch341a_regs[pInformation->USBwValues.bw.bb1];
    
    if (length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = sizeof(ch341a_reg);
        return NULL;
    }
    return (uint8_t*)ch341a_reg;
}

static void req_write_reg(uint32_t addr, uint32_t value) {
    DBG_VAL("write_reg(addr = 0x", addr, 16, ")");
    DBG_VAL("  value = 0x", value, 16, "");
    
    if (addr < sizeof(ch341a_regs))
        ch341a_regs[addr] = value;
    if (addr == CH341A_REG_DIVISOR || addr == CH341A_REG_PRESCALER ||
        addr == CH341A_REG_LCR)
        serial_init();
}

static void modem_ctrl(void) {
    uint16_t control = pInformation->USBwValues.w;
    DBG_VAL("modem_ctrl(control = 0b", control, 2, ")");
    modem_control = (control >> 8);
    update_mode();
}

static void send_data_if_needed(void) {
    static uint8_t buf[CH341A_DATA_SIZE];
    uint32_t rx_len;

    if (usb_tx_in_progress != 0)
        return;
    
    rx_len = (sizeof(rx_buf) + rx_ptr_end - rx_ptr_begin) % sizeof(rx_buf);
    if (rx_len == 0)
        return;

    DBG_VAL("send_data_if_needed(rx_len = ", rx_len, 10, ")");
    
    {
        uint32_t to_send = rx_len;
        uint32_t max_to_send, i;
        
        update_mode();
        switch (ch341a_mode) {
        default:
        case Mode_UART:
            max_to_send = CH341A_DATA_SIZE;
            break;
        case Mode_SPI:
            max_to_send = CH341A_DATA_SIZE - 1;
            if (spi_tx_in_progress != 0 && rx_len < max_to_send)
                return;
            break;
        }
        
        if (to_send > max_to_send)
            to_send = max_to_send;

        for (i = 0; i < to_send; i++) {
            buf[i] = rx_buf[rx_ptr_begin];
            rx_ptr_begin = (rx_ptr_begin + 1) % sizeof(rx_buf);
        }

        if (ch341a_mode == Mode_SPI) {
            STM_ARR(" (spi >> usb) ", (const char*)buf, to_send, "");
        }
        else if (ch341a_mode == Mode_UART) {
            STM_ARR(" (uart >> usb) ", (const char*)buf, to_send, "");
        }
        else {
            STM_ARR(" (none >> usb) ", (const char*)buf, to_send, "");
        }

        USB_SIL_Write(ENDP2, buf, to_send);
        SetEPTxValid(ENDP2);
        usb_tx_in_progress = 1;
    }
}

static void update_usb_rx_state(void) {
    uint32_t tx_len = (sizeof(tx_buf) + tx_ptr_end - tx_ptr_begin) % sizeof(tx_buf);
    if (tx_len + CH341A_DATA_SIZE < sizeof(tx_buf)) {
        SetEPRxValid(ENDP2);
        if (usb_rx_in_progress == 0) {
            INF_VAL("resume usb reception, tx_len = ", tx_len, 10, "");
            usb_rx_in_progress = 1;
        }
    }
    else {
        SetEPTxStatus(ENDP2, EP_TX_NAK);
        if (usb_rx_in_progress != 0) {
            INF_VAL("pause usb reception, tx_len = ", tx_len, 10, "");
            usb_rx_in_progress = 0;
        }
    }
}

static void schedule_transmission(const uint8_t *buf, uint32_t length) {
    uint32_t i;
    for (i = 0; i < length; i++) {
        uint32_t next_tx_ptr_end = (tx_ptr_end + 1) % sizeof(tx_buf);
        if (next_tx_ptr_end == tx_ptr_begin) { /* overflow */
            WRN_STR("transmit buffer overflowed");
            break;
        }
        tx_buf[tx_ptr_end] = buf[i];
        tx_ptr_end = next_tx_ptr_end;
    }
}

static uint16_t spi_prescaler_by_speed(uint32_t need_speed, uint32_t *real_speed) {
    RCC_ClocksTypeDef rcc;
    uint32_t spi_pclk, approx;
    uint16_t ret;
    RCC_GetClocksFreq(&rcc);
    
    if (SPI_SPI == SPI1)
        spi_pclk = rcc.PCLK1_Frequency;
    else
        spi_pclk = rcc.PCLK2_Frequency;
    
    approx = (spi_pclk + need_speed / 2) / need_speed;
    
    if (approx < 3) {
        ret = SPI_BaudRatePrescaler_2;
        *real_speed = spi_pclk / 2;
    }
    else if (approx < 6) {
        ret = SPI_BaudRatePrescaler_4;
        *real_speed = spi_pclk / 4;
    }
    else if (approx < 12) {
        ret = SPI_BaudRatePrescaler_8;
        *real_speed = spi_pclk / 8;
    }
    else if (approx < 24) {
        ret = SPI_BaudRatePrescaler_16;
        *real_speed = spi_pclk / 16;
    }
    else if (approx < 48) {
        ret = SPI_BaudRatePrescaler_32;
        *real_speed = spi_pclk / 32;
    }
    else if (approx < 96) {
        ret = SPI_BaudRatePrescaler_64;
        *real_speed = spi_pclk / 64;
    }
    else if (approx < 192) {
        ret = SPI_BaudRatePrescaler_128;
        *real_speed = spi_pclk / 128;
    }
    else {
        ret = SPI_BaudRatePrescaler_256;
        *real_speed = spi_pclk / 256;
    }
    
    return ret;
}

static void do_i2c_stream(const uint8_t *buf, uint8_t length) {
    uint32_t i;
    for (i = 0; i < length; i++) {
        uint32_t cmd = buf[i];
        DBG_VAL("do_i2c_stream(cmd = 0x", cmd, 16, ")");
        if ((cmd & CH341A_CMD_I2C_STM_SET) == CH341A_CMD_I2C_STM_SET) {
            uint32_t speed = cmd & CH341A_CMD_I2C_STM_SPEED_MASK;
            uint32_t width = cmd & CH341A_CMD_I2C_STM_WIDTH_MASK;
            uint32_t real_speed;
            switch (speed) {
            case CH341A_CMD_I2C_STM_SPEED_20KHZ:
                speed = 20000;
                break;
            case CH341A_CMD_I2C_STM_SPEED_100KHZ:
                speed = 100000;
                break;
            case CH341A_CMD_I2C_STM_SPEED_400KHZ:
                speed = 400000;
                break;
            case CH341A_CMD_I2C_STM_SPEED_750KHZ:
                speed = 750000;
                break;
            default:
                WRN_VAL("unknown i2c speed code: 0x", speed, 16, ", ignoring command");
                return;
            }
            DBG_VAL("  requested speed: ", speed, 10, " bps");
            if (ch341a_spi_fixed_speed > 0)
                speed = ch341a_spi_fixed_speed;
            else if (ch341a_spi_speed_factor > 0)
                speed *= ch341a_spi_speed_factor;
            DBG_VAL("  modified requested speed: ", speed, 10, " bps");
            
            switch (width) {
            case CH341A_CMD_I2C_STM_WIDTH_SINGLE:
                width = 8;
                break;
            case CH341A_CMD_I2C_STM_WIDTH_DOUBLE:
                width = 16;
                break;
            default:
                WRN_VAL("unknown i2c width code: 0x", width, 16, ", ignoring command");
                return;
            }
            DBG_VAL("  width: ", width, 10, " b");

            {
                SPI_InitTypeDef s;
                SPI_StructInit(&s);
                s.SPI_Mode = SPI_Mode_Master;
                s.SPI_DataSize = (width == 8 ? SPI_DataSize_8b : SPI_DataSize_16b);
                s.SPI_NSS = SPI_NSS_Soft;
                s.SPI_BaudRatePrescaler = spi_prescaler_by_speed(speed, &real_speed);
                s.SPI_FirstBit = SPI_FirstBit_LSB;
                SPI_Init(SPI_SPI, &s);
            }
            
            DBG_VAL("  real_speed: ", real_speed, 10, " bps");
            
            INF() {
                console_putstr("spi settings: baudrate=");
                console_putnum(real_speed, 10, 0);
                console_putstr("\r\n");
            }
            
            SPI_I2S_ITConfig(SPI_SPI, SPI_I2S_IT_RXNE, ENABLE);
    
            NVIC_PriorityGroupConfig(IRQ_PRIO_GROUP_CFG);
            {
                NVIC_InitTypeDef s;
                s.NVIC_IRQChannel = SPI_IRQ;
                s.NVIC_IRQChannelPreemptionPriority = CH341A_IO_IRQ_PRIO;
                s.NVIC_IRQChannelSubPriority = 0;
                s.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&s);
            }
        }
    }
}

static void do_uio_stream(const uint8_t *buf, uint8_t length) {
    uint32_t i;
    
    DBG_ARR(" do_uio_stream(commands = ", (const char*)buf, length, ")");
    
    for (i = 0; i < length; i++) {
        uint32_t cmd = buf[i];
        if ((cmd & CH341A_CMD_UIO_STM_OUT) == CH341A_CMD_UIO_STM_OUT) {
            BitAction cs_state = (cmd & 1) ? Bit_SET : Bit_RESET;
            DBG_VAL("  switch CS bit to ", (uint32_t)cs_state, 10, "");
            GPIO_WriteBit(SPI_GPIO, SPI_CS, cs_state);
        }
        if ((cmd & CH341A_CMD_UIO_STM_DIR) == CH341A_CMD_UIO_STM_DIR) {
            if (cmd & 0x3F) {
                INF_STR("Enable SPI");
                spi_enabled = 1;
                update_mode();
                SPI_Cmd(SPI_SPI, ENABLE);
            }
            else {
                INF_STR("Disable SPI");
                spi_enabled = 0;
                update_mode();
                SPI_Cmd(SPI_SPI, DISABLE);
                SetEPTxCount(ENDP2, 0);
                SetEPTxStatus(ENDP2, EP_TX_NAK);
                usb_tx_in_progress = 0;
                spi_tx_in_progress = 0;
                tx_ptr_begin = tx_ptr_end = 0;
                rx_ptr_begin = rx_ptr_end = 0;
            }
        }
    }
}

static void handle_command(const uint8_t *buf, uint32_t length) {
    if (length == 0)
        return;
    
    switch (buf[0]) {
    case CH341A_CMD_SPI_STREAM:
        buf++;
        length--;
        STM_ARR(" (usb >> spi) ", (const char*)buf, length, "");
        schedule_transmission(buf, length);
        if (length > 0 && spi_tx_in_progress == 0)
            ch341a_spi_irq();
        break;
    case CH341A_CMD_I2C_STREAM:
        while (length > 1 && buf[length - 1] != CH341A_CMD_I2C_STM_END)
            length--;
        if (length == 1) {
            WRN_ARR("can't find CH341A_CMD_I2C_STM_END in ", (const char*)buf, length, "");
        }
        else
            do_i2c_stream(buf + 1, length - 1);
        break;
    case CH341A_CMD_UIO_STREAM:
        while (length > 1 && buf[length - 1] != CH341A_CMD_UIO_STM_END)
            length--;
        if (length == 1) {
            WRN_ARR("can't find CH341A_CMD_UIO_STM_END ", (const char*)buf, length, "");
        }
        else
            do_uio_stream(buf + 1, length - 1);
        break;
    default:
        WRN_ARR("unhandled command ", (const char*)buf, length, "");
        break;
    }
}

static void process_received_data(const uint8_t *buf, uint32_t length) {
    DBG_VAL("process_received_data(length = ", length, 10, ")");
    
    update_mode();
    
    switch (ch341a_mode) {
    case Mode_UART:
        STM_ARR(" (usb >> uart) ", (const char*)buf, length, "");
        schedule_transmission(buf, length);
        if (length > 0)
            USART_ITConfig(SERIAL1_USART, USART_IT_TXE, ENABLE);
        break;
    case Mode_None:
    case Mode_SPI:
        handle_command(buf, length);
        break;
    default:
        break;
    }
}

static void ch341a_init(void) {
    uint32_t i;
    INF_STR("ch341a init()");

    pInformation->Current_Configuration = 0;
    usbd_power_on();
    USB_SIL_Init();

    tx_ptr_begin = tx_ptr_end = ch341a_tx_total = 0;
    rx_ptr_begin = rx_ptr_end = ch341a_rx_total = 0;
    for (i = 0; i < sizeof(tx_buf); i++)
        tx_buf[i] = 0;
    for (i = 0; i < sizeof(rx_buf); i++)
        rx_buf[i] = 0;

    usb_tx_in_progress = 0;
    spi_tx_in_progress = 0;
    modem_control = ~0;
    spi_enabled = 0;

    update_mode();
}

static void ch341a_reset(void) {
    INF_STR("ch341a reset");
    
    pInformation->Current_Configuration = 0;
    pInformation->Current_Feature = CH341A_ConfigDescriptor[7];
    pInformation->Current_Interface = 0;
    SetBTABLE(BTABLE_ADDRESS);

    /* Initialize Endpoint 0 */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);

    /* Initialize Endpoint 1 */
    SetEPType(ENDP1, EP_INTERRUPT);
    SetEPTxAddr(ENDP1, ENDP1_TXADDR);
    SetEPTxStatus(ENDP1, EP_TX_NAK);
    SetEPRxStatus(ENDP1, EP_RX_DIS);

    /* Initialize Endpoint 2 */
    SetEPType(ENDP2, EP_BULK);
    SetEPTxAddr(ENDP2, ENDP2_TXADDR);
    SetEPRxAddr(ENDP2, ENDP2_RXADDR);
    SetEPTxCount(ENDP2, 0);
    SetEPRxCount(ENDP2, CH341A_DATA_SIZE);
    SetEPTxStatus(ENDP2, EP_TX_NAK);
    SetEPRxStatus(ENDP2, EP_RX_VALID);
    
    usb_tx_in_progress = 0;
    usb_rx_in_progress = 1;
    spi_tx_in_progress = 0;

    /* Set this device to response on default address */
    SetDeviceAddress(0);
}

static void ch341a_status_in(void) {
    DBG_STR("status_in()");
}

static void ch341a_status_out(void) {
    DBG_STR("status_out()");
}

static RESULT ch341a_data_setup(uint8_t RequestNo) {
    uint8_t    *(*CopyRoutine)(uint16_t);

    CopyRoutine = NULL;

    DBG_VAL("data_setup(RequestNo = 0x", RequestNo, 16, ")");
    DBG_VAL("  Type_Recipient = 0x", Type_Recipient, 16, ")");
    DBG_VAL("  USBwValues  = 0x", pInformation->USBwValues.w, 16, ")");
    DBG_VAL("  USBwIndexs  = 0x", pInformation->USBwIndexs.w, 16, ")");
    DBG_VAL("  USBwLengths = 0x", pInformation->USBwLengths.w, 16, ")");
    
    if (Type_Recipient == (VENDOR_REQUEST | DEVICE_RECIPIENT)) {
        switch (RequestNo) {
        case CH341A_REQ_READ_VERSION:
            CopyRoutine = req_read_version;
            break;
        case CH341A_REQ_READ_REG:
            CopyRoutine = req_read_reg;
            break;
        default:
            break;
        }
    }
    
    if (CopyRoutine == NULL)
        return USB_UNSUPPORT;

    pInformation->Ctrl_Info.CopyData = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)(0);
    return USB_SUCCESS;
}

static RESULT ch341a_nodata_setup(uint8_t RequestNo) {
    DBG_VAL("nodata_setup(RequestNo = 0x", RequestNo, 16, ")");
    DBG_VAL("  Type_Recipient = 0x", Type_Recipient, 16, ")");
    DBG_VAL("  USBwValues = 0x", pInformation->USBwValues.w, 16, ")");
    DBG_VAL("  USBwIndexs = 0x", pInformation->USBwIndexs.w, 16, ")");

    if (Type_Recipient == (VENDOR_REQUEST | DEVICE_RECIPIENT)) {
        switch (RequestNo) {
        case CH341A_REQ_WRITE_REG:
            req_write_reg(pInformation->USBwValues.bw.bb0, pInformation->USBwIndexs.bw.bb0);
            req_write_reg(pInformation->USBwValues.bw.bb1, pInformation->USBwIndexs.bw.bb1);
            return USB_SUCCESS;
        case CH341A_REQ_SERIAL_INIT:
            serial_init();
            return USB_SUCCESS;
        case CH341A_REQ_MODEM_CTRL:
            modem_ctrl();
            return USB_SUCCESS;
        default:
            break;
        }
    }
    return USB_UNSUPPORT;
}

static RESULT ch341a_get_interface_setting(uint8_t Interface, uint8_t AlternateSetting) {
    DBG_VAL("get_interface_setting(Interface = 0x", Interface, 16, ")");
    if (AlternateSetting > 0)
        return USB_UNSUPPORT;
    else if (Interface > 1)
        return USB_UNSUPPORT;
    return USB_SUCCESS;
}

static uint8_t *ch341a_get_device_descriptor(uint16_t Length) {
    DBG_VAL("get_device_descriptor(Length = ", Length, 10, ")");
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

static uint8_t *ch341a_get_config_descriptor(uint16_t Length) {
    DBG_VAL("get_config_descriptor(Length = ", Length, 10, ")");
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

static uint8_t *ch341a_get_string_descriptor(uint16_t Length) {
    uint8_t wValue0 = pInformation->USBwValue0;
    DBG_VAL("get_string_descriptor(wValue0 = 0x", wValue0, 16, ")");
    return NULL;
}

static inline void push_to_rx_buf(uint8_t ch) {
    uint32_t next_rx_ptr_end = (rx_ptr_end + 1) % sizeof(rx_buf);
    if (next_rx_ptr_end != rx_ptr_begin) {
        rx_buf[rx_ptr_end] = ch;
        rx_ptr_end = next_rx_ptr_end;
        ch341a_rx_total++;
    }
}

static inline int pop_from_tx_buf(void) {
    tx_ptr_begin = (tx_ptr_begin + 1) % sizeof(tx_buf);
    ch341a_tx_total++;
    return (tx_ptr_begin == tx_ptr_end) ? 1 : 0;
}

void ch341a_serial_irq() {
    if (USART_GetFlagStatus(SERIAL1_USART, USART_FLAG_TXE) != RESET) {
        if (ch341a_mode == Mode_UART && tx_ptr_begin != tx_ptr_end) {
            USART_SendData(SERIAL1_USART, tx_buf[tx_ptr_begin]);
            if (pop_from_tx_buf())
                USART_ITConfig(SERIAL1_USART, USART_IT_TXE, DISABLE);
        }
        else
            USART_ITConfig(SERIAL1_USART, USART_IT_TXE, DISABLE);
    }
    if (USART_GetFlagStatus(SERIAL1_USART, USART_FLAG_RXNE) != RESET) {
        uint32_t ch = USART_ReceiveData(SERIAL1_USART);
        if (ch341a_mode == Mode_UART)
            push_to_rx_buf(ch);
    }
}

void ch341a_spi_irq(void) {
    if (SPI_I2S_GetFlagStatus(SPI_SPI, SPI_I2S_FLAG_RXNE) != RESET) {
        uint32_t ch = SPI_I2S_ReceiveData(SPI_SPI);
        if (ch341a_mode == Mode_SPI)
            push_to_rx_buf(ch);
    }
    if (ch341a_mode == Mode_SPI && tx_ptr_begin != tx_ptr_end) {
        spi_tx_in_progress = 1;
        SPI_I2S_SendData(SPI_SPI, tx_buf[tx_ptr_begin]);
        pop_from_tx_buf();
    }
    else
        spi_tx_in_progress = 0;
}

void ch341a_on_ep1_in(void) {
    DBG_STR("on_ep1_in()");
}

void ch341a_on_packet_transmitted() {
    usb_tx_in_progress = 0;
    send_data_if_needed();
}

void ch341a_on_packet_received(const uint8_t *buf, uint8_t length) {
    process_received_data((uint8_t*)buf, length);
    update_usb_rx_state();
}

void ch341a_on_frame_interval() {
    if (ch341a_mode == Mode_SPI && tx_ptr_begin != tx_ptr_end && spi_tx_in_progress == 0)
        ch341a_spi_irq();
    send_data_if_needed();
    update_usb_rx_state();
}
