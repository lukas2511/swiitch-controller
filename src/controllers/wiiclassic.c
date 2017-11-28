#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>

#include "i2c.h"
#include "controller.h"
#include "bitset.h"

uint8_t controller_state[20] = {0, 0, 0, 0, 0, 0};
uint8_t controller_state_bytes = 6;

int poll_controller(void) {
  i2c_write(0x52, (uint8_t[]){0x00}, 1);

  if(i2c_read(0x52, controller_state, 6) == 6) {
    switch_controller.data.RX = (_GETBITS(controller_state[0], 0, 2) << 3 | _GETBITS(controller_state[1], 0, 2) << 1 | _GETBITS(controller_state[2], 0, 1)) << 3;
    switch_controller.data.RY = _GETBITS(controller_state[2], 3, 5) << 3;

    switch_controller.data.LX = _GETBITS(controller_state[0], 2, 6) << 2;
    switch_controller.data.LY = _GETBITS(controller_state[1], 2, 6) << 2;

    switch_controller.data.A = _GETBIT(controller_state[5], 3);
    switch_controller.data.B = _GETBIT(controller_state[5], 1);
    switch_controller.data.X = _GETBIT(controller_state[5], 4);
    switch_controller.data.Y = _GETBIT(controller_state[5], 2);
    switch_controller.data.plus = _GETBIT(controller_state[4], 5);
    switch_controller.data.minus = _GETBIT(controller_state[4], 3);
    switch_controller.data.home = _GETBIT(controller_state[4], 4);
    switch_controller.data.dpad_up = _GETBIT(controller_state[5], 7);
    switch_controller.data.dpad_down = _GETBIT(controller_state[4], 1);
    switch_controller.data.dpad_left = _GETBIT(controller_state[5], 6);
    switch_controller.data.dpad_right = _GETBIT(controller_state[4], 0);
    switch_controller.data.L = _GETBIT(controller_state[4], 2);
    switch_controller.data.R = _GETBIT(controller_state[4], 6);
    switch_controller.data.ZL = _GETBIT(controller_state[5], 0);
    switch_controller.data.ZR = _GETBIT(controller_state[5], 5);

    // switch has no analog triggers, if it had those would be the values
    // LT = (_GETBITS(controller_state[2], 1, 2) << 3 | _GETBITS(controller_state[3], 0, 3)) << 3
    // RT = _GETBITS(controller_state[3], 3, 5) << 3
    return 1;
  }
  return 0;
}

void init_controller(void) {
  init_i2c();

  // try to disable encryption
  i2c_write(0x52, (uint8_t[]){0xF0, 0x55}, 2);
  i2c_write(0x52, (uint8_t[]){0xFB, 0x00}, 2);
}
