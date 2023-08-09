#include "driver_3ep.h"

void emulate_cartridge(uint8_t* cart_rom, uint8_t* cart_ram) {
	emulate_cartridge_generic(cart_rom, 32 * 1024, cart_ram);
}
