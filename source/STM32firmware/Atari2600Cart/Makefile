# Use the official ARM Cortex GNU toolchain
#     (http//developer.arm.com/open-source/gnu-toolchain/gnu-rm)
# and the open source stlink tool suite
#     (https://github.com/texane/stlink)
# to build and flash the firmware with this Makefile.
#
# Makefile derived from https://github.com/nitsky/stm32-example

BUILDDIR = build
DEPDIR = .depend/build

CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

STFLASH = st-flash

SOURCES = \
	src/startup_stm32f40xx.s \
	src/system_stm32f4xx.c \
	Libraries/tm_stm32f4_delay/tm_stm32f4_delay.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c \
	Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c \
	Libraries/tm_stm32f4_gpio/tm_stm32f4_gpio.c \
	Libraries/tm_stm32f4_fatfs/tm_stm32f4_fatfs.c \
	Libraries/tm_stm32f4_fatfs/fatfs/diskio.c \
	Libraries/tm_stm32f4_fatfs/fatfs/option/ccsbcs.c \
	Libraries/tm_stm32f4_fatfs/fatfs/drivers/fatfs_sd_sdio.c \
	Libraries/tm_stm32f4_fatfs/fatfs/option/syscall.c \
	Libraries/tm_stm32f4_fatfs/fatfs/ff.c \
	src/cartridge_firmware.c \
	src/cartridge_supercharger.c \
	src/cartridge_3f.c \
	src/cartridge_3e.c \
	src/cartridge_3ep.c \
	src/cartridge_bf.c \
	src/cartridge_ace.c \
	src/cartridge_pp.c \
	src/cartridge_df.c \
	src/cartridge_dpc.c \
	src/flash.c \
	src/main.c

INCLUDES = \
	-Isrc \
	-ILibraries/CMSIS/Include \
	-ILibraries/Device/STM32F4xx/Include \
	-ILibraries/STM32F4xx_StdPeriph_Driver/inc \
	-ILibraries/tm_stm32f4_fatfs \
	-ILibraries/tm_stm32f4_fatfs/fatfs \
	-ILibraries/tm_stm32f4_fatfs/fatfs/drivers \
	-ILibraries/tm_stm32f4_delay \
	-ILibraries/tm_stm32f4_gpio

# MovieCart backend (src/moviecart, from moviecart/firmware/stm32). Its own SDIO
# driver and globals would clash with UnoCart's, so it is linked into one
# relocatable object that exports only moviecart_launch().
MC_SOURCES = \
	src/moviecart/mc_main.c \
	src/moviecart/core.c \
	src/moviecart/bus_service.c \
	src/moviecart/sd_reader.c \
	src/moviecart/mc_sd_sdio.c \
	src/moviecart/pff.c \
	src/moviecart/frame.c \
	src/moviecart/update.c \
	src/moviecart/mclog.c \
	src/moviecart/title_data.c

GARBAGE = $(DEPDIR) $(BUILDDIR)

OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(SOURCES))))
MC_OBJECTS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(MC_SOURCES))))
MC_OBJECT = $(BUILDDIR)/moviecart.o

ELF = $(BUILDDIR)/firmware.elf
HEX = $(BUILDDIR)/firmware.hex
BIN = $(BUILDDIR)/firmware.bin
MAP = $(BUILDDIR)/firmware.map

CFLAGS = \
	-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d \
	-O2 -fno-tree-vrp -g \
	-mcpu=cortex-m4 -mthumb \
	-mfpu=fpv4-sp-d16 -mhard-float \
	-DSTM32F4XX \
	-DSTM32F40XX \
	-DUSE_STM32F4_DISCOVERY \
	-DHSE_VALUE=8000000 \
	-DUSE_STDPERIPH_DRIVER \
	-ffunction-sections \
	-fdata-sections \
	$(INCLUDES)

# The MovieCart build's flags; core.c's extra -fno-* keep its dispatch code in
# the shape the bus timing was measured with.
MC_CFLAGS = \
	-MT $@ -MMD -MP -MF $(DEPDIR)/$*.d \
	-DMOVIECART_NO_LOG=1 \
	-DMOVIECART_NO_LED=1 \
	-ffreestanding -fno-common \
	-O2 -fno-tree-vrp -g \
	-mcpu=cortex-m4 -mthumb \
	-mfpu=fpv4-sp-d16 -mhard-float \
	-DSTM32F4XX \
	-DSTM32F40XX \
	-DUSE_STM32F4_DISCOVERY \
	-DHSE_VALUE=8000000 \
	-DUSE_STDPERIPH_DRIVER \
	-ffunction-sections \
	-fdata-sections \
	-Isrc/moviecart \
	$(INCLUDES)

$(BUILDDIR)/src/moviecart/core.o: MC_CFLAGS += -fno-gcse -fno-crossjumping -fno-toplevel-reorder

LDSCRIPT = stm32f4_flash.ld
LDFLAGS += \
	-T$(LDSCRIPT) \
	-mthumb -mcpu=cortex-m4 \
	-mhard-float \
	--specs=nosys.specs \
	-Wl,--gc-sections \
	-Wl,-Map=$(MAP)

bin: $(BIN)
hex: $(HEX)
elf: $(ELF)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(ELF): $(OBJECTS) $(MC_OBJECT) $(LDSCRIPT)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(MC_OBJECT) $(LDLIBS)

$(MC_OBJECT): $(MC_OBJECTS)
	$(LD) -r -nostdlib -o $@.tmp $(MC_OBJECTS)
	$(OBJCOPY) --keep-global-symbol=moviecart_launch $@.tmp $@
	rm -f $@.tmp

$(MC_OBJECTS): $(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	mkdir -p .depend/$(dir $@)
	$(CC) -c $(MC_CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.c
	mkdir -p $(dir $@)
	mkdir -p .depend/$(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILDDIR)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

flash: $(BIN)
	$(STFLASH) write $(BIN) 0x8000000

clean:
	rm -rf $(GARBAGE)

.PHONY: bin elf hex flash clean

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES) $(MC_SOURCES))))
