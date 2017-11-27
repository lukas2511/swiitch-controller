#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>

#include "include/wiiclassic.h"
#include "include/i2c.h"

union WiiClassicController wiiclassic_controller;

int poll_wiiclassic(void) {
	return i2c_read(0x52, wiiclassic_controller.bytes, 6);
}

void init_wiiclassic(void) {
	init_i2c();

	// try to disable encryption
	i2c_write(0x52, (uint8_t[]){0xF0, 0x55}, 2);
	i2c_write(0x52, (uint8_t[]){0xFB, 0x00}, 2);
}
