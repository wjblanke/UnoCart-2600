#include <stdbool.h>
#include <stdint.h>

#include "cartridge_io.h"

void emulate_cartridge(uint8_t* cart_rom, uint8_t* cart_ram) {
	uint16_t addr = 0, addr_prev = 0, data = 0, data_prev = 0;

	while (1)
	{
		while ((addr = ADDR_IN) != addr_prev)
			addr_prev = addr;
		// got a stable address
		if (addr & 0x1000)
		{ // A12 high
			if (addr & 0x0f00) {
				DATA_OUT = ((uint16_t)cart_rom[addr & 0x0fff]) << 8;
				SET_DATA_MODE_OUT
				// wait for address bus to change
				while (ADDR_IN == addr) ;
				SET_DATA_MODE_IN
			} else if (addr & 0x0080) {
				DATA_OUT = ((uint16_t)cart_ram[addr & 0x007f]) << 8;
				SET_DATA_MODE_OUT;
				while (ADDR_IN == addr);
				SET_DATA_MODE_IN;
			} else {
				while (ADDR_IN == addr) { data_prev = data; data = DATA_IN; }
				cart_ram[addr & 0x007f] =  data_prev >> 8;
			}
		}
	}

	__enable_irq();
}
