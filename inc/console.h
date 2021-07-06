#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "config.h"
#include "hw_config.h"

#define CONSOLE_LVL_DEBUG   0
#define CONSOLE_LVL_STREAMS 1
#define CONSOLE_LVL_INFO    2
#define CONSOLE_LVL_MSG     3
#define CONSOLE_LVL_WARN    4
#define CONSOLE_LVL_ERR     5

#define CONSOLE_STR(level, str) { \
    if (console_start_record((level), CONSOLE_MAX_MSG_LEN)) { \
        console_putstr((str)); \
        console_putstr("\r\n"); \
    } \
}
#define CONSOLE_VAL(level, prefix, value, base, suffix) { \
    if (console_start_record((level), CONSOLE_MAX_MSG_LEN)) { \
        console_putstr((prefix)); \
        console_putnum((value), (base), 0); \
        console_putstr((suffix)); \
        console_putstr("\r\n"); \
    } \
}
#define CONSOLE_ARR(level, prefix, arr, length, suffix) { \
    if (console_start_record((level), CONSOLE_MAX_MSG_LEN)) { \
        console_putstr((prefix)); \
        console_putstr("["); \
        console_putnum((length), 10, 0); \
        console_putstr("] "); \
        console_putasc((arr), ':', (length)); \
        console_putstr((suffix)); \
        console_putstr("\r\n"); \
    } \
}

#define DBG() if (console_start_record(CONSOLE_LVL_DEBUG, CONSOLE_MAX_MSG_LEN))
#define DBG_STR(str) CONSOLE_STR(CONSOLE_LVL_DEBUG, (str))
#define DBG_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_DEBUG, (prefix), (value), (base), (suffix))
#define DBG_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_DEBUG, (prefix), (arr), (length), (suffix))

#define STM() if (console_start_record(CONSOLE_LVL_STREAMS, CONSOLE_MAX_MSG_LEN))
#define STM_STR(str) CONSOLE_STR(CONSOLE_LVL_STREAMS, (str))
#define STM_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_STREAMS, (prefix), (value), (base), (suffix))
#define STM_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_STREAMS, (prefix), (arr), (length), (suffix))

#define INF() if (console_start_record(CONSOLE_LVL_INFO, CONSOLE_MAX_MSG_LEN))
#define INF_STR(str) CONSOLE_STR(CONSOLE_LVL_INFO, (str))
#define INF_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_INFO, (prefix), (value), (base), (suffix))
#define INF_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_INFO, (prefix), (arr), (length), (suffix))

#define MSG() if (console_start_record(CONSOLE_LVL_MSG, CONSOLE_MAX_MSG_LEN))
#define MSG_STR(str) CONSOLE_STR(CONSOLE_LVL_MSG, (str))
#define MSG_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_MSG, (prefix), (value), (base), (suffix))
#define MSG_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_MSG, (prefix), (arr), (length), (suffix))

#define WRN() if (console_start_record(CONSOLE_LVL_WARN, CONSOLE_MAX_MSG_LEN))
#define WRN_STR(str) CONSOLE_STR(CONSOLE_LVL_WARN, (str))
#define WRN_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_WARN, (prefix), (value), (base), (suffix))
#define WRN_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_WARN, (prefix), (arr), (length), (suffix))

#define ERR() if (console_start_record(CONSOLE_LVL_ERR, CONSOLE_MAX_MSG_LEN))
#define ERR_STR(str) CONSOLE_STR(CONSOLE_LVL_ERR, (str))
#define ERR_VAL(prefix, value, base, suffix) CONSOLE_VAL(CONSOLE_LVL_ERR, (prefix), (value), (base), (suffix))
#define ERR_ARR(prefix, arr, length, suffix) CONSOLE_ARR(CONSOLE_LVL_ERR, (prefix), (arr), (length), (suffix))


void console_init(int enable_echo, int min_level);
int console_level_allowed(int level);

void console_flush(void);

void console_putraw(const char *buf, uint32_t nbytes);
void console_putstr(const char *s);
void console_putasc(const char *buf, char delimiter, uint32_t nbytes);
void console_putnum(uint32_t value, int base, uint32_t min_width);
void console_putint(int32_t value);

int console_start_record(int level, uint32_t min_space_in_txbuf);

uint32_t console_bytes_available(void);
uint32_t console_read(char *msg, uint32_t max_length);
uint32_t console_readline(char *msg, uint32_t max_length);

void console_irq(void);

#endif /* __CONSOLE_H */
