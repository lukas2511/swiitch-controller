#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>

#include "include/wiiclassic.h"
#include "include/i2c.h"
#include "include/controller.h"
#include "bitset.h"

int poll_wiiclassic(void) {
  uint8_t buf[6];
  if(i2c_read(0x52, buf, 6) == 6) {
    switch_controller.data.RX = (_GETBITS(buf[0], 0, 2) << 3 | _GETBITS(buf[1], 0, 2) << 1 | _GETBITS(buf[2], 0, 1)) << 3;
    switch_controller.data.RY = _GETBITS(buf[2], 3, 5) << 3;

    switch_controller.data.LX = _GETBITS(buf[0], 2, 6) << 2;
    switch_controller.data.LY = _GETBITS(buf[1], 2, 6) << 2;

    switch_controller.data.A = _GETBIT(buf[5], 3);
    switch_controller.data.B = _GETBIT(buf[5], 1);
    switch_controller.data.X = _GETBIT(buf[5], 4);
    switch_controller.data.Y = _GETBIT(buf[5], 2);
    switch_controller.data.plus = _GETBIT(buf[4], 5);
    switch_controller.data.minus = _GETBIT(buf[4], 3);
    switch_controller.data.home = _GETBIT(buf[4], 4);
    switch_controller.data.dpad_up = _GETBIT(buf[5], 7);
    switch_controller.data.dpad_down = _GETBIT(buf[4], 1);
    switch_controller.data.dpad_left = _GETBIT(buf[5], 6);
    switch_controller.data.dpad_right = _GETBIT(buf[4], 0);
    switch_controller.data.L = _GETBIT(buf[4], 2);
    switch_controller.data.R = _GETBIT(buf[4], 6);
    switch_controller.data.ZL = _GETBIT(buf[5], 0);
    switch_controller.data.ZR = _GETBIT(buf[5], 5);

    // switch has no analog triggers, if it had those would be the values
    // LT = (_GETBITS(buf[2], 1, 2) << 3 | _GETBITS(buf[3], 0, 3)) << 3
    // RT = _GETBITS(buf[3], 3, 5) << 3
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
