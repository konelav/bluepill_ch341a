/*
 * Here you can customize firmware.
 */

/***********************************
 * Verbose name of hardware platform, only used for console output
 * when booting up.
 */
#define PLATFORM_NAME               "bluepill stm32f103c8t6 (cortex-m3)"

/***********************************
 * Console settings.
 * Console subsystem has two buffers: for transmission of log messages
 * and for reception of commands.
 * Both transmission and reception are done in USART interrupt handler,
 * buffers are required. Transmission buffer should be big enough to
 * contain block of consecutive debug lines or command output,
 * and reception buffer can be much smaller (no bigger than longest
 * command).
 * When some log record requested and transmission buffer has less
 * than CONSOLE_MAX_MSG_LEN bytes free, then record is skipped.
 * Level of logging can be changed at runtime via console commands,
 * CONSOLE_MIN_LEVEL refers to default level at startup.
 */
#define CONSOLE_BAUDRATE            460800
#define CONSOLE_TX_BUFSIZE          2048
#define CONSOLE_RX_BUFSIZE          64
#define CONSOLE_MAX_MSG_LEN         160
#define CONSOLE_ENABLE_ECHO         1
#define CONSOLE_MIN_LEVEL           CONSOLE_LVL_INFO

/***********************************
 * If LED_INVERT is set to 1 then `led_on()` pushs gpio pin low and
 * `led_off()` pulls it high.
 */
#define LED_INVERT                  1

/***********************************
 * SPI baudrate default settings.
 * flashrom sends command to set SPI baudrate to 100kHz, which is quite
 * slow.
 *   - if you wish to have fixed baudrate and ignore commands
 *     for its changes, set SPI_FIXED_SPEED to desired positive value;
 *   - otherwise requested baudrate will be multiplied by some factor;
 *   - there are three modes and three factor values:
 *     HI (default at startup), LO and TURBO;
 *   - switching between factor modes is done by console commands.
 */
#define SPI_FIXED_SPEED             0
#define SPI_TURBO_SPEED_FACTOR      100
#define SPI_HI_SPEED_FACTOR         10
#define SPI_LO_SPEED_FACTOR         1

/***********************************
 * Period of LED blinks when device in different modes,
 * in microseconds.
 */
#define BLINK_MODE_NONE             (2000 * 1000)
#define BLINK_MODE_UART             (500 * 1000)
#define BLINK_MODE_SPI              (100 * 1000)

/***********************************
 * Period between reports with generic information about device state,
 * in miscoseconds.
 * Generic information is: current mode, total number of bytes
 * transmitted and received (both SPI and UART).
 * New report is generated when byte totals changed, but
 * respecting these limits.
 */ 
#define MIN_REPORT_PERIOD           (1 * 1000000)
#define MAX_REPORT_PERIOD           (10 * 1000000)

/***********************************
 * Device USB idVendor and idProduct.
 * It can be changed for some reason, e.g. ch341.ko driver from 
 * linux kernel 4.9 searchs for PIDs 0x5523, 0x7523, and not 0x5512.
 */
#define CH341A_VID                  0x1A86
#define CH341A_PID                  0x5512

/***********************************
 * Miscellaneous settings of ch341 specific behaviour.
 * 
 * CH341A_MAX_PACKET_SIZE, CH341A_DATA_SIZE and CH341A_INT_SIZE
 *   are USB-specific sizes (maximum transfer size and endpoint data
 *   sizes).
 * 
 * CH341A_MAX_PACKETS is used for calculating internal buffer size
 *   for queueing data received from SPI/UART and scheduled for
 *   transmission to SPI/UART. Buffer size is
 *         (CH341A_DATA_SIZE * CH341A_MAX_PACKETS)
 * 
 * CH341A_FRAME_INTERVAL is number of USB Start-Of-Frame (SOF) tokens
 *   received between checks if transmission needed; in this case
 *   state of IN endpoint is changed.
 * 
 * CH341A_ALWAYS_ENABLE_RXTX when it is set to 1, both RX and TX mode
 *   for UART is used, ignoring flags from LCR register; for example,
 *   old ch341.ko driver from 4.9 linux kernel sets only RX flag.
 */
#define CH341A_MAX_PACKET_SIZE      8
#define CH341A_DATA_SIZE            32
#define CH341A_INT_SIZE             8
#define CH341A_MAX_PACKETS          256
#define CH341A_FRAME_INTERVAL       2
#define CH341A_ALWAYS_ENABLE_RXTX   1
