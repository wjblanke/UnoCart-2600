#include <string.h>

#include "cartridge_firmware.h"

#include "firmware_pal_rom.h"
#include "firmware_pal60_rom.h"
#include "firmware_ntsc_rom.h"

static unsigned char menu_ram[1024];	// < NUM_DIR_ITEMS * 12
static char menu_status[16];
static unsigned const char *firmware_rom = firmware_ntsc_rom;

void set_menu_status_msg(const char* message) {
	strncpy(menu_status, message, 15);
}

void set_menu_status_byte(char status_byte) {
	menu_status[15] = status_byte;
}

void set_tv_mode(int tv_mode) {
	switch (tv_mode) {
		case TV_MODE_NTSC:
			firmware_rom = firmware_ntsc_rom;
			break;

		case TV_MODE_PAL:
			firmware_rom = firmware_pal_rom;
			break;

		case TV_MODE_PAL60:
			firmware_rom = firmware_pal60_rom;
			break;
	}
}

uint8_t* get_menu_ram() {
	return menu_ram;
}

// We require the menu to do a write to $1FF4 to unlock the comms area.
// This is because the 7800 bios accesses this area on console startup, and we wish to ignore these
// spurious reads until it has started the cartridge in 2600 mode.
bool comms_enabled = false;

/* Drive data for the whole A12-high window (like a real cart OE).
 * Menu commands are LDA $1E00,X (item index in low address bits) — require a
 * few identical samples before accepting, or noisy A0-A7 selects the wrong file.
 */
static int cart_cmd_stable(uint16_t *addr_out) {
	uint16_t a0 = ADDR_IN;
	if ((a0 & 0x1F00) != 0x1E00)
		return 0;
	uint16_t a1 = ADDR_IN;
	uint16_t a2 = ADDR_IN;
	if (a0 == a1 && a1 == a2 && (a2 & 0x1F00) == 0x1E00) {
		*addr_out = a2;
		return 1;
	}
	return 0;
}

int emulate_firmware_cartridge() {
	__disable_irq();	// Disable interrupts
	uint16_t addr;
	while (1)
	{
		addr = ADDR_IN;
		if (addr & 0x1000)
		{ // A12 high
			if (comms_enabled)
			{	// normal mode, once the cartridge code has done its init.
				// on a 7800, we know we are in 2600 mode now.
				if (cart_cmd_stable(&addr))
					break;
				if (addr >= 0x1800 && addr < 0x1C00)
					DATA_OUT_SET(((uint16_t)menu_ram[addr&0x3FF])<<8);
				else if ((addr & 0x1FF0) == CART_STATUS_BYTES)
					DATA_OUT_SET(((uint16_t)menu_status[addr&0xF])<<8);
				else
					DATA_OUT_SET(((uint16_t)firmware_rom[addr&0xFFF])<<8);
				SET_DATA_MODE_OUT
				while (ADDR_IN & 0x1000) {
					addr = ADDR_IN;
					if (cart_cmd_stable(&addr))
						goto got_cmd;
					if (addr >= 0x1800 && addr < 0x1C00)
						DATA_OUT_SET(((uint16_t)menu_ram[addr&0x3FF])<<8);
					else if ((addr & 0x1FF0) == CART_STATUS_BYTES)
						DATA_OUT_SET(((uint16_t)menu_status[addr&0xF])<<8);
					else
						DATA_OUT_SET(((uint16_t)firmware_rom[addr&0xFFF])<<8);
				}
				SET_DATA_MODE_IN
			}
			else
			{	// prior to an access to $1FF4, we might be running on a 7800 with the CPU at
				// ~1.8MHz so we've got less time than usual - keep this short.
				DATA_OUT_SET(((uint16_t)firmware_rom[addr&0xFFF])<<8);
				SET_DATA_MODE_OUT
				while (ADDR_IN & 0x1000) {
					addr = ADDR_IN;
					DATA_OUT_SET(((uint16_t)firmware_rom[addr&0xFFF])<<8);
					if (addr == 0x1FF4)
						comms_enabled = true;
				}
				SET_DATA_MODE_IN
			}
		}
	}

got_cmd:
	SET_DATA_MODE_IN
	__enable_irq();
	return addr;
}

bool reboot_into_cartridge() {
	set_menu_status_byte(1);

	return emulate_firmware_cartridge() == CART_CMD_START_CART;
}
