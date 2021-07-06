CH341 chip emulator based on Bluepill board
============================================

What is this
------------

This is tiny firmware for stm32f10x MCU that partially mimics behaviour
of USB-to-many converter [ch341]. It supposed to be run on
so-called [bluepill] widely available board that has 
[stm32f10x] on it.
This project tries to achieve best possible simplicity of
configuring, building and flashing firmware, hence it has:
  
  - no external library dependencies, only basic and simpliest are used
    and included to source tree in somewhat *stripped* form;
  - only most widely available cross-platform tools are needed
    for building and/or flashing.


Main purposes
-------------

There are several ones:

  1. To have 3.3V SPI programmer for NOR flash EEPROM chips compatible
  with [flashrom] tool via USB interface. Although it can
  be achieved with original [ch341] chip, most popular board has
  flawed schematics with 5V on MOSI/MISO/SCK lines hence it can damage
  memory chip and other 3.3V equipment (parts of motherboard, for
  example).
  
  2. To have 5V-tolerant 3.3V USB-to-UART converter compatible with
  [ch341.ko] driver from mainline linux kernel.
  
  3. The core code does almost nothing, simply resending bytes from
  USB to SPI or UART periphery and vice versa; still, it has some useful
  generic subsystems such as 32-bit system timer, multilevel logger with
  simple console and basic USB device routines, and rather flexible
  generalized structure, so it can be good *template project*, i.e. used 
  as starting point for bigger one or for education.


Configuring
-----------

Look into `config.h` file for configurable properties.


Building
--------

Used libraries (all included in source tree in `libs/` directory):

  - [cmsis], including startup and `SystemInit()` routines
    for stm32f10x medium density MCUs;
  - [stdperiph] Standard Peripherals Library
    for stm32f10x family;
  - [usbfs] USB Full Speed Device library for STM MCUs.

Build dependencies:

  - [arm-none-eabi-gcc] (tested with 5.4.1);
  - [arm-none-eabi-binutils] (tested with 2.28);
  - [make] (tested with GNU Make 4.1).

Simply run within the root directory:

```
    $ make
```

It will create three files:

  - `stm32f10x.elf` - ELF-formatted image; suitable for flashing with
  [openocd] utility, for example;
  - `stm32f10x.bin` - raw binary image for loading to device's flash
  memory, usually starting from `0x08000000` address; suitable for
  uploading with [openocd] or 
  [stm32flash] utilities;
  - `stm32f10x.hex` - binary image in intel HEX format for loading
  to device's flash memory; suitable for uploading with
  [stm32flash] utility.


Flashing
--------

You can use any feasible way for uploading firmware images to MCU.
However there are some helper targets in `Makefile`. They use simple
`bash` scripts.

For OpenOCD utility (tested with version 0.9.0):

  - `make program_jlink_swd`: program with SEGGER [jlink]
  compatible programmer in SWD mode (most suitable for bluepill board);
  - `make program_jlink_jtag`: program with SEGGER [jlink]
  compatible programmer in JTAG mode;
  - `make program_stlink`: program with [stlink] v2
  programmer;
  - `make program`: same as `make program_jlink_swd`.

For STM32Flash utility (tested with version 0.6):

  - `make upload_stm32flash`: program with serial interface; you need to
  switch BOOT0 to `1` (the one that is **not** near `RESET` button
  on bluepill board), connect 3.3V or 5V UART to pins A9 (TX) and
  A10 (RX); by default `/dev/ttyUSB0` used as UART device, it can be
  changed e.g. `make STM32FLASH_DEV=/dev/ttyS1 upload_stm32flash`.

Also there are helper targets for downloading firmware image from MCU
and uploading it back. Again, either `openocd` and J-Link/ST-Link
programmer or `stm32flash` and UART is needed:

  - `make download_jlink_swd`, `make download_jlink_jtag`,
  `make download_stlink`, `make download_stm32flash`;
  - `make upload_jlink_swd`, `make upload_jlink_jtag`,
  `make upload_stlink`, `make upload_stm32flash`.


Pinout
------

Pinout can be changed using `hw_config.h` file.
By default it is as follows:

PIN | Subsystem  | Function
----|------------|---------
C13 | LED        | Signals about current mode by blink speed
A11 | USB        | Data Minus
A12 | USB        | Data Plus
B12 | CH341.SPI  | Chip Select (NSS)
B13 | CH341.SPI  | Clock (SCK)
B14 | CH341.SPI  | Master Input Slave Output (MISO)
B15 | CH341.SPI  | Master Output Slave Input (MOSI)
A9  | CH341.UART | Transmitter (TX, this is *output* of MCU)
A10 | CH341.UART | Receiver (RX, this is *input* of MCU)
B10 | CONSOLE(*) | Transmitter of misc messages (TX, this is *output* of MCU)
B11 | CONSOLE(*) | Receiver of commads (RX, this is *input* of MCU)

(*) You can connect any UART (3.3V or 5V TTL) to `CONSOLE` interface and
see what is going on and even execute some simple commands. Default
console baudrate is 460800, it can be changed in `config.h` file.
Example of usage:

```
    $ picocom /dev/ttyUSB0 -b 460800
```

Then type `help` to see list of available commands.

You can even connect one bluepill board flashed with this firmware 
to another one (`A10:A9:GND -> B10:B11:GND`) to access console of
the latter, or use latter to program former (with `A10:A9` pins
and BOOT0 switched to `1`) with `stm32flash` utility.


Compatibility
-------------

SPI mode tested with `flashrom` version 1.2, by successfully
reading and writing contents of Windbond [w25x64] and Macronix
[mx25l3205] (replacing BIOS with [libreboot] on
Thinkpad X200). This was done with the following steps:

  1. Connect J-Link OB ARM Emulator [jlink_swd] USB programmer to PC
  and to bluepill (`DIO`, `DCLK`, `3.3`, `GND`).
  
  2. Run `make program` within the root of the project.
  
  3. Connect bluepill to [pomona] clip following SOIC-8 or
  SOIC-16 pinout (described e.g. here [libreboot-flash], 
  `CS->B12`, `SCK->B13`, `MOSI->B15`, `MISO->B14`, `3.3`, `GND`).
  
  4. Connect bluepill to PC with microUSB cable.
  
  5. Run `flashrom -p ch341a_spi -r bios_backup.bin`.
  
  6. With `spispeed:turbo` mode, 8mb flash read/written in 38 seconds.


UART mode tested with `ch341.ko` driver from linux kernel 5.10, by
connecting UART lines (`RX` and `TX`) of programmed bluepill to
[ft232rl] or [pl2303] USB-to-UART converter, then
starting two [picocom] sessions on baudrates up to 460800
and checking connectivity.


Issues
------

1. I2C and Parallel interfaces are not implemented.

2. Neither setting nor getting of modem lines status
   (CTS, RTS, DTR, DSR, RI, DCD) is implemented.

3. UART baudrates above 460800 could experience problems with
   prescaler/divisor setup leading to wrong real baudrates.


TODO
----

1. Modem lines CTS, RTS, DTR, DSR, RI, DCD:

    - allocate GPIO pins;
    - change status by request (`modem_ctrl()`);
    - poll and report state changes.

2. Use DMA instead of interrupts for logging/console subsystem.

3. Test building and using with MS Windows.

4. Implement I2C interface.

5. Implement Parallel interface.


[ch341]: http://wch-ic.com/products/CH341.html
[bluepill]: https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill.html
[stm32f10x]: https://www.st.com/resource/en/reference_manual/cd00171190-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
[flashrom]: https://www.flashrom.org/Flashrom
[ch341.ko]: https://github.com/torvalds/linux/blob/master/drivers/usb/serial/ch341.c
[cmsis]: https://github.com/ARM-software/CMSIS/blob/master/Device/_Template_Flash/Test/system_stm32f10x.c
[stdperiph]: https://www.st.com/en/embedded-software/stsw-stm32054.html
[usbfs]: https://www.st.com/en/embedded-software/stsw-stm32121.html
[arm-none-eabi-gcc]: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
[arm-none-eabi-binutils]: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm
[make]: https://www.gnu.org/software/make/
[openocd]: https://github.com/ntfreak/openocd
[stm32flash]: https://sourceforge.net/projects/stm32flash/
[jlink]: https://www.segger.com/products/debug-probes/j-link/
[stlink]: https://www.st.com/en/development-tools/st-link-v2.html
[w25x64]: https://www.winbond.com/hq/product/code-storage-flash-memory/serial-nor-flash/
[mx25l3205]: https://www.macronix.com/
[libreboot]: https://libreboot.org/
[jlink_swd]: https://www.arduino-tech.com/the-j-link-ob-arm-emulator-debugger-programmer-downloader-jlink-instead-of-v8-swd/
[libreboot-flash]: https://libreboot.org/docs/install/spi.html
[pomona]: https://www.pomonaelectronics.com/products/test-clips/ic-test-clips
[ft232rl]: https://ftdichip.com/wp-content/uploads/2020/08/DS_FT232R.pdf
[pl2303]: http://www.prolific.com.tw/US/ShowProduct.aspx?pcid=41
