<img width="4000" height="3000" alt="PXL_20260809_193644739" src="https://github.com/user-attachments/assets/c3d5af26-9860-44f7-9cf4-6c502741c74e" />

# UnoCart-2600 (DevEBox fork)

Fork of [DirtyHairy/UnoCart-2600](https://github.com/DirtyHairy/UnoCart-2600) (originally by Robin Edwards / electrotrains), adapted for the **[MCUDEV DevEBox STM32F407VGT6](https://github.com/mcauser/MCUDEV_DEVEBOX_F407VGT6)** board and its onboard microSD socket.

Upstream releases and history remain useful background; this tree is **not** drop-in compatible with the classic STM32F4 Discovery + breakout wiring.

## Changes in this fork

### Target hardware

- Primary target: **MCUDEV DevEBox STM32F407VGT6** (8 MHz HSE, AMS1117 3.3 V regulator, onboard microSD via SDIO).
- Cartridge breakout wiring must match the remapped buses below (not the original Discovery PD=address / PE=data map).

### Cartridge bus remap

To free **PD2** for SDIO CMD (and avoid fighting the Atari address bus), the cart buses were moved:

| Bus | Original (Discovery) | This fork |
|-----|----------------------|-----------|
| Address A0–A12 | PD0–PD12 | **PE0–PE12** |
| Data D0–D7 | PE8–PE15 | **PD8–PD15** |
| Video mode straps | PC0 / PC1 | Unchanged (PC0=PAL60, PC1=PAL) |

Wiring checklist (Atari cart → MCU):

| Atari | MCU |
|-------|-----|
| A0–A12 | PE0–PE12 |
| D0–D7 | PD8–PD15 |
| +5V / GND | Board 5V input / GND (MCU runs at 3.3 V; level-shift or series resistors as in your breakout) |

GPIO details (`cartridge_io.h`, `main.c` in both `Atari2600Cart` and `standalone`):

- `SET_DATA_MODE_IN` / `SET_DATA_MODE_OUT` only change **PD8–PD15** MODER bits so **PD0–PD7** (including SDIO CMD on PD2) stay intact during cart emulation.
- `DATA_OUT_SET()` writes only the high byte of GPIOD ODR so SDIO pins are not cleared every bus cycle.

### Menu bus timing (A12-gated drive)

Upstream menu emulation waited for two identical `ADDR_IN` samples, then drove data only until the address changed exactly. On breadboard / long-wire setups that makes lower address bits noisy enough that the MCU rarely (or never) drives **PD8–PD15**, so the TV stays blank even when **PE12 (A12)** looks high.

This fork’s menu ROM path drives data for the whole **A12-high** window and updates the byte as the address changes (closer to a real cart OE). Menu **commands** (`LDA $1E00,X` to select item X) still require a few identical address samples so noisy A0–A7 do not open the wrong file. Implemented in `cartridge_firmware.c` (`emulate_firmware_cartridge`).

Game-cart drivers still use the older address-stability loops; if a title glitches after selection, the same A12-gated approach may need to be applied there.

### SD card: SPI → SDIO 4-bit

Upstream used SPI (SPI2 on PB13/14/15, CS PB5). The DevEBox microSD is wired for **SDIO**, so FatFs now uses the SDIO driver:

| SD pin | MCU |
|--------|-----|
| D0 | PC8 |
| D1 | PC9 |
| D2 | PC10 |
| D3 | PC11 |
| CK | PC12 |
| CMD | PD2 |

Defines in `source/STM32firmware/Atari2600Cart/src/defines.h`:

- `FATFS_USE_SDIO 1`
- `FATFS_SDIO_4BIT 1`

Makefile builds `fatfs_sd_sdio.c` plus `stm32f4xx_sdio`, `stm32f4xx_dma`, and `misc` (SPI SD path removed from the CLI build).

Init runs 1-bit then switches to 4-bit wide bus (falls back to 1-bit if wide-bus fails). Transfer clock defaults to ~8 MHz (`SDIO_TRANSFER_CLK_DIV` in `defines.h`) for reliability on the onboard socket; init stays ~400 kHz. `disk_initialize` and directory mounts retry a few times if the first attempt fails.

The DevEBox microSD **card-detect pin is NC** — `FATFS_USE_DETECT_PIN` is forced off so a floating PB6 is not mistaken for “no card.”

### GPIO pull-ups (SDIO)

Internal pull-ups are enabled on **D0–D3** and **CMD**. **CK (PC12)** is configured with **no pull** (host-driven clock).

### Clock

System clock is **HSI → PLL at 168 MHz** (SDIOCLK 48 MHz via PLLQ). This avoids depending on the Discovery-oriented HSE setup; the DevEBox has an 8 MHz HSE, but this firmware does not require it for SYSCLK.

### Building

From `source/STM32firmware/Atari2600Cart` with an Arm GNU toolchain (`arm-none-eabi-gcc`, with newlib). A Darwin aarch64 toolchain may be unpacked under `tools/` in this repo for local builds:

```bash
export PATH="/path/to/arm-none-eabi/bin:$PATH"
cd source/STM32firmware/Atari2600Cart
make clean
make bin hex    # outputs in build/firmware.bin and build/firmware.hex
```

Flash with ST-Link (`make flash` / `st-flash`) or DFU (BOOT0) as you prefer on the DevEBox.

### Compatibility notes

- **Not compatible** with stock Discovery UnoCart breakouts without rewiring address/data to PE / PD8–15.
- Onboard DevEBox W25Q16 SPI flash and TFT header are unused by this firmware; they must not conflict with your cart wiring.
- Keep cart wiring short and grounded well; noisy A0–A11 was the main failure mode before the menu bus-timing change.
- SD card: FAT or FAT32, as upstream.

---

The remainder of this README is adapted from the [original repository](https://github.com/robinhedwards/UnoCart-2600) / [DirtyHairy firmware tree](https://github.com/DirtyHairy/UnoCart-2600).

UnoCart-2600
============
The UnoCart-2600 is an open source SD-card multicart for the Atari 2600. Use the joystick or the SELECT/RESET keys
to navigate the SD card and select a title to play.

SD card should be formatted as FAT or FAT32.

The UnoCart-2600 can emulate most banking schemes with ROM sizes up to 64k and RAM sizes up to 32k.
(more description to follow)

![Image](images/UnoCart2600Disco.jpg?raw=true)

Hardware
--------
The original UnoCart-2600 was based on Robin Edwards’ earlier UnoCart project for the Atari 8-bit. Upstream builds typically used an STM32F407 DISCOVERY board
connected to a small PCB to breakout the Atari 2600 cartridge signals.

An article describing how to build an UnoCart for the Atari 8-bit was published in
[Excel Magazine](http://excel-retro-mag.co.uk) issue #4. You can also get a PDF of the article [here](https://github.com/robinhedwards/UnoCart/blob/master/UnoCart_EXCEL4.pdf).

Building the cartridge for the 2600 is almost identical, with the same connections between D0-D7 and A0-A12, +5V and GND.
All the other connections to the breakout PCB can be skipped, since these signals are not present on the 2600 cartridge slot.

Obviously, you'll need a breakout board designed for the 2600 cartridge slot rather than the Atari 8-bit. The design files for the breakout PCB are hosted here and can be used to make your own copy of the PCB.

**This fork:** use the DevEBox pin map in [Changes in this fork](#changes-in-this-fork) above, not the Discovery pin map from the original article.

The UnoCart-2600 menu can be set to display in NTSC, PAL or PAL60 as follows:
* By default, the firmware will be NTSC.
* Connect PC0 -> GND for PAL60
* Connect PC1 -> GND for PAL

Note that this is for the menu only, and has no effect after you have selected a cartridge to play.

![Image](images/menuPAL.jpg?raw=true)

Custom PCB
----------
Robin Edwards also designed a custom PCB for the UnoCart-2600 pictured here. This is mainly to make it easier to get people
to test the design. The PCB design files are not currently public.

![Front of PCB when inserted in Atari](images/test_board_front_small.jpg?raw=true)
![Back of PCB when inserted in Atari](images/test_board_back_small.jpg?raw=true)

Firmware
--------
The UnoCart-2600 firmware is open source under a GPL license. Upstream firmware updates: [DirtyHairy releases](https://github.com/DirtyHairy/UnoCart-2600/releases).

Credits
-------
* Design, hardware and firmware by Robin Edwards (electrotrains at atariage)
* Additional work on firmware by Christian Speckner (DirtyHairy at atariage)
* This fork: DevEBox STM32F407VGT6 port (SDIO microSD, remapped cart buses, A12-gated menu bus drive)
