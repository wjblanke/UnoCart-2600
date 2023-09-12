#include <stdbool.h>
#include <stdint.h>

#include "cartridge_io.h"

#define STARTUP_BANK_BFSC 15

void emulate_cartridge(uint8_t* cart_rom, uint8_t* cart_ram) {
	uint16_t addr = 0, addr_prev = 0, addr_prev2 = 0, data = 0, data_prev = 0;
	uint8_t *bank = cart_rom + STARTUP_BANK_BFSC * 0x1000;

	while (1)
	{
		while (((addr = ADDR_IN) != addr_prev) || (addr != addr_prev2))
		{
			addr_prev2 = addr_prev;
			addr_prev = addr;
		}

        if ((addr & 0x1000) == 0) continue;

		uint16_t address = addr & 0x0fff;

        if (address >= 0x0f80 && address <= 0x0fbf)
			bank = cart_rom + 0x1000 * (address & 0x007f);

		if (address & 0x0f00) {
			DATA_OUT = ((uint16_t)bank[address]) << 8;
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

	__enable_irq();
}
