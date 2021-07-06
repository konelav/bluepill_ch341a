#include "console.h"
#include "timer.h"

static volatile uint8_t tx_buf[CONSOLE_TX_BUFSIZE];
static volatile uint32_t tx_ptr_begin = 0, tx_ptr_end = 0;

static volatile uint8_t rx_buf[CONSOLE_RX_BUFSIZE];
static volatile uint32_t rx_ptr_begin = 0, rx_ptr_end = 0;

static int ENABLE_ECHO = 0, MIN_LEVEL = 0;

static uint32_t messages_skipped = 0;


static inline uint8_t byte_to_char(uint8_t value) {
    if (value < 10)
        return '0' + value;
    return 'A' + (value - 10);
}

void console_init(int enable_echo, int min_level) {
    ENABLE_ECHO = enable_echo;
    MIN_LEVEL = min_level;
    
    NVIC_DisableIRQ(CONSOLE_IRQ);
    
    {
        USART_InitTypeDef s;
        s.USART_BaudRate = CONSOLE_BAUDRATE;
        s.USART_WordLength = USART_WordLength_8b;
        s.USART_StopBits = USART_StopBits_1;
        s.USART_Parity = USART_Parity_No;
        s.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        s.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(CONSOLE_USART, &s);
    }
    
    tx_ptr_begin = tx_ptr_end = 0;
    rx_ptr_begin = rx_ptr_end = 0;
    
    USART_ITConfig(CONSOLE_USART, USART_IT_RXNE, ENABLE);
    
    NVIC_PriorityGroupConfig(IRQ_PRIO_GROUP_CFG);
    {
        NVIC_InitTypeDef s;
        s.NVIC_IRQChannel = CONSOLE_IRQ;
        s.NVIC_IRQChannelPreemptionPriority = CONSOLE_IRQ_PRIO;
        s.NVIC_IRQChannelSubPriority = 0;
        s.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&s);
    }
    
    USART_Cmd(CONSOLE_USART, ENABLE);
}

int console_level_allowed(int level) {
    return (level < MIN_LEVEL) ? 0 : 1;
}

static inline ErrorStatus console_put_char(uint8_t ch) {
    ErrorStatus res = ERROR;
    uint32_t next_tx_ptr_end = (tx_ptr_end + 1) % sizeof(tx_buf);
    if (next_tx_ptr_end != tx_ptr_begin) { /* no overflow */
        tx_buf[tx_ptr_end] = ch;
        tx_ptr_end = next_tx_ptr_end;
        res = SUCCESS;
        USART_ITConfig(CONSOLE_USART, USART_IT_TXE, ENABLE);
    }
    return res;
}

void console_flush(void) {
    while (tx_ptr_begin != tx_ptr_end)
        ;
}

void console_putraw(const char *buf, uint32_t nbytes) {
    uint32_t i;
    for (i = 0; i < nbytes; i++)
        console_put_char(buf[i]);
}

void console_putstr(const char *s) {
    while (*s) {
        console_put_char(*s);
        s++;
    }
}

void console_putasc(const char *buf, char delimiter, uint32_t nbytes) {
    uint32_t i;
    for (i = 0; i < nbytes; i++) {
        console_put_char(byte_to_char((uint8_t)buf[i] >> 4));
        console_put_char(byte_to_char((uint8_t)buf[i] & 0x0f));
        if (delimiter && i < nbytes - 1)
            console_put_char((uint8_t)delimiter);
    }
}

void console_putnum(uint32_t value, int base, uint32_t min_width) {
    uint8_t buf[33];
    uint32_t len = 0, i;
    if (value == 0)
        buf[len++] = '0';
    else {
        while (value > 0) {
            uint32_t digit = value % base;
            value /= base;
            buf[len++] = byte_to_char(digit);
        }
    }
    for (i = len; i < min_width; i++)
        console_put_char((uint8_t)'0');
    while (len > 0) {
        len--;
        console_put_char(buf[len]);
    }
}

void console_putint(int32_t value) {
    if (value < 0) {
        console_putstr("-");
        value = -value;
    }
    console_putnum((uint32_t)value, 10, 0);
}

int console_start_record(int level, uint32_t min_space_in_txbuf) {
    uint32_t bytes_in_txbuf, ts;
    if (level < MIN_LEVEL)
        return 0;
    bytes_in_txbuf = (sizeof(tx_buf) + tx_ptr_end - tx_ptr_begin) % sizeof(tx_buf);
    if (sizeof(tx_buf) - bytes_in_txbuf < min_space_in_txbuf) {
        messages_skipped++;
        return 0;
    }
    ts = timer_usec();
    if (messages_skipped > 0) {
        console_putstr(" (");
        console_putnum(messages_skipped, 10, 0);
        console_putstr(" record(s) skipped)\r\n");
        messages_skipped = 0;
    }
    console_putstr("[");
    console_putnum(ts / 1000000, 10, 0);
    console_putstr(".");
    console_putnum(ts % 1000000, 10, 6);
    console_putstr("] ");
    return 1;
}

uint32_t console_bytes_available(void) {
    return (sizeof(rx_buf) + rx_ptr_end - rx_ptr_begin) % sizeof(rx_buf);
}

uint32_t console_read(char *msg, uint32_t max_length) {
    uint32_t available = console_bytes_available();
    uint32_t to_read = (max_length < available) ? max_length : available;
    uint32_t i;
    for (i = 0; i < to_read; i++) {
        msg[i] = rx_buf[rx_ptr_begin];
        rx_ptr_begin = (rx_ptr_begin + 1) % sizeof(rx_buf);
    }
    return to_read;
}

uint32_t console_readline(char *msg, uint32_t max_length) {
    uint32_t available = console_bytes_available();
    uint32_t to_read, i;
    if (available < 1)
        return 0;
    for (to_read = 0; to_read < available; to_read++) {
        uint8_t ch = rx_buf[(rx_ptr_begin + to_read) % sizeof(rx_buf)];
        if (ch == '\r')
            break;
    }
    if (to_read == available)
        return 0;
    if (to_read >= max_length)
        to_read = max_length - 1;
    for (i = 0; i < to_read + 1; i++) {
        msg[i] = rx_buf[rx_ptr_begin];
        rx_ptr_begin = (rx_ptr_begin + 1) % sizeof(rx_buf);
    }
    msg[to_read] = '\0';
    return to_read;
}

void console_irq(void) {
    if (USART_GetFlagStatus(CONSOLE_USART, USART_FLAG_TXE) != RESET) {
        if (tx_ptr_begin != tx_ptr_end) {
            USART_SendData(CONSOLE_USART, tx_buf[tx_ptr_begin]);
            tx_ptr_begin = (tx_ptr_begin + 1) % sizeof(tx_buf);
        }
        else
            USART_ITConfig(CONSOLE_USART, USART_IT_TXE, DISABLE);
    }
    if (USART_GetFlagStatus(CONSOLE_USART, USART_FLAG_RXNE) != RESET) {
        uint32_t ch = USART_ReceiveData(CONSOLE_USART);
        rx_buf[rx_ptr_end] = ch;
        rx_ptr_end = (rx_ptr_end + 1) % sizeof(rx_buf);
        if (rx_ptr_end == rx_ptr_begin) {
            /* overflow, data lost */
            rx_ptr_begin = (rx_ptr_begin + 1) % sizeof(rx_buf);
        }
        if (ENABLE_ECHO)
            console_put_char(ch);
    }
}
