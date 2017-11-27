#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>

#include "include/wiiclassic.h"
#include "include/i2c.h"
#include "include/controller.h"

#define GETBITS(c, start, len) (c << start >> (8-len))
#define GETBIT(c, n) GETBITS(c, n, 1)

int poll_wiiclassic(void) {
  uint8_t buf[6];
  if(i2c_read(0x52, buf, 6) == 6) {
    switch_controller.data.RX = (GETBITS(buf[0], 0, 2) << 3 | GETBITS(buf[1], 0, 2) << 1 | GETBITS(buf[2], 0, 1)) << 3;
    switch_controller.data.RY = GETBITS(buf[2], 3, 5) << 3;

    switch_controller.data.LX = GETBITS(buf[0], 2, 6) << 2;
    switch_controller.data.LY = GETBITS(buf[1], 2, 6) << 2;

    switch_controller.data.A = GETBIT(buf[5], 3);
    switch_controller.data.B = GETBIT(buf[5], 1);
    switch_controller.data.X = GETBIT(buf[5], 4);
    switch_controller.data.Y = GETBIT(buf[5], 2);
    switch_controller.data.plus = GETBIT(buf[4], 5);
    switch_controller.data.minus = GETBIT(buf[4], 3);
    switch_controller.data.home = GETBIT(buf[4], 4);
    switch_controller.data.dpad_up = GETBIT(buf[5], 7);
    switch_controller.data.dpad_down = GETBIT(buf[4], 1);
    switch_controller.data.dpad_left = GETBIT(buf[5], 6);
    switch_controller.data.dpad_right = GETBIT(buf[4], 0);
    switch_controller.data.L = GETBIT(buf[4], 2);
    switch_controller.data.R = GETBIT(buf[4], 6);
    switch_controller.data.ZL = GETBIT(buf[5], 0);
    switch_controller.data.ZR = GETBIT(buf[5], 5);

    // switch has no analog triggers, if it had those would be the values
    // LT = (GETBITS(buf[2], 1, 2) << 3 | GETBITS(buf[3], 0, 3)) << 3
    // RT = GETBITS(buf[3], 3, 5) << 3
    return 1;
  }
  return 0;
}

void init_wiiclassic(void) {
  init_i2c();

  // try to disable encryption
  i2c_write(0x52, (uint8_t[]){0xF0, 0x55}, 2);
  i2c_write(0x52, (uint8_t[]){0xFB, 0x00}, 2);
}
