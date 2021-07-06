TARGET = stm32f10x
TOOLPREFIX = arm-none-eabi-
STM32FLASH_DEV = /dev/ttyUSB0

DEFINES = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

ARCH = -mcpu=cortex-m3 -mthumb
COPTS = $(ARCH) -O2 -std=gnu99 -Wall
LOPTS = --gc-sections

CC = $(TOOLPREFIX)gcc $(COPTS) $(DEFINES)
LD = $(TOOLPREFIX)ld  $(LOPTS)
OC = $(TOOLPREFIX)objcopy

OBJ_DIR = obj

LDSCRIPTS = ldscripts
CMSIS = libs/CMSIS
STDPERIPH = libs/STM32F10x_StdPeriph_Driver
USBFS = libs/STM32_USB-FS-Device_Driver
APP = .

INCLUDES = inc $(CMSIS)/inc $(STDPERIPH)/inc $(USBFS)/inc
INCOPT = $(addprefix -I,$(INCLUDES))

# order matters
LDSCR = $(LDSCRIPTS)/libs.ld $(LDSCRIPTS)/mem.ld $(LDSCRIPTS)/sections.ld
SCROPT = $(addprefix -T,$(LDSCR))

STARTUP = startup_stm32f10x_md.s
SYSINIT = system_stm32f10x.c

HEADERS = $(shell find . -name '*.h' | grep -v fwinfo.h)

CMSIS_OBJS = $(OBJ_DIR)/cmsis/$(STARTUP).o $(OBJ_DIR)/cmsis/$(SYSINIT).o

STDPERIPH_SRC = $(shell find $(STDPERIPH)/src -name '*.c')
_STDPERIPH_OBJ1 = $(STDPERIPH_SRC:.c=.o)
_STDPERIPH_OBJ2 = $(notdir $(_STDPERIPH_OBJ1))
STDPERIPH_OBJS = $(addprefix $(OBJ_DIR)/stdperiph/, $(_STDPERIPH_OBJ2))

USBFS_SRC = $(shell find $(USBFS)/src -name '*.c')
_USBFS_OBJ1 = $(USBFS_SRC:.c=.o)
_USBFS_OBJ2 = $(notdir $(_USBFS_OBJ1))
USBFS_OBJS = $(addprefix $(OBJ_DIR)/usbfs/, $(_USBFS_OBJ2))

APP_SRC = $(shell find $(APP)/src -name '*.c')
_APP_OBJ1 = $(APP_SRC:.c=.o)
_APP_OBJ2 = $(notdir $(_APP_OBJ1))
APP_OBJS = $(addprefix $(OBJ_DIR)/app/, $(_APP_OBJ2))


default: $(TARGET).bin $(TARGET).hex


$(OBJ_DIR)/cmsis/$(STARTUP).o: $(CMSIS)/src/$(STARTUP)
	@mkdir -p $(OBJ_DIR)/cmsis
	$(CC) -c -o $@ $(CMSIS)/src/$(STARTUP)
$(OBJ_DIR)/cmsis/$(SYSINIT).o: $(HEADERS) $(CMSIS)/src/$(SYSINIT)
	@mkdir -p $(OBJ_DIR)/cmsis
	$(CC) $(INCOPT) -c -o $@ $(CMSIS)/src/$(SYSINIT)

$(OBJ_DIR)/stdperiph/%.o: $(STDPERIPH)/src/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)/stdperiph
	$(CC) $(INCOPT) -c -o $@ $<

$(OBJ_DIR)/usbfs/%.o: $(USBFS)/src/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)/usbfs
	$(CC) $(INCOPT) -c -o $@ $<

$(OBJ_DIR)/app/%.o: $(APP)/src/%.c $(HEADERS)
	@mkdir -p $(OBJ_DIR)/app
	bash scripts/set_fwinfo.sh
	$(CC) $(INCOPT) -c -o $@ $<

$(TARGET).elf: $(CMSIS_OBJS) $(STDPERIPH_OBJS) $(USBFS_OBJS) $(APP_OBJS) $(LDSCR)
	$(LD) -o $@ $(SCROPT) $(CMSIS_OBJS) $(STDPERIPH_OBJS) $(USBFS_OBJS) $(APP_OBJS)
$(TARGET).bin: $(TARGET).elf
	$(OC) -O binary $< $@
$(TARGET).hex: $(TARGET).elf
	$(OC) -O ihex $< $@

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex

program_jlink_swd: $(TARGET).elf
	bash ./scripts/do_program.sh ./openocd-jlink-swd.cfg $(TARGET).elf
program_jlink_jtag: $(TARGET).elf
	bash ./scripts/do_program.sh ./openocd-jlink-jtag.cfg $(TARGET).elf
program_stlink: $(TARGET).elf
	bash ./scripts/do_program.sh ./openocd-stlink.cfg $(TARGET).elf
program: program_jlink_swd

download_jlink_swd:
	bash ./scripts/do_download.sh ./openocd-jlink-swd.cfg
download_jlink_jtag:
	bash ./scripts/do_download.sh ./openocd-jlink-jtag.cfg
download_stlink:
	bash ./scripts/do_download.sh ./openocd-stlink.cfg
download_stm32flash:
	stm32flash -r $(TARGET)_downloaded.hex $(STM32FLASH_DEV)
download: download_jlink_swd

upload_jlink_swd: $(TARGET).bin
	bash ./scripts/do_upload.sh ./openocd-jlink-swd.cfg $(TARGET).bin
upload_jlink_jtag: $(TARGET).bin
	bash ./scripts/do_upload.sh ./openocd-jlink-jtag.cfg $(TARGET).bin
upload_stlink: $(TARGET).bin
	bash ./scripts/do_upload.sh ./openocd-stlink.cfg $(TARGET).bin
upload_stm32flash: $(TARGET).hex
	stm32flash -w $(TARGET).hex -v -g 0x0 $(STM32FLASH_DEV)
upload: upload_jlink_swd

udev_rules:
	bash scripts/make_udev_rules.sh
