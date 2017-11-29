#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

#include "controller.h"
#include <librfn/fibre.h>
#include <librfn/time.h>
#include "bitset.h"

#define SNES_CLOCK GPIO2
#define SNES_LATCH GPIO1
#define SNES_DATA  GPIO0

// the snes controller is missing some important features
// on the first run of snescontroller_poller it detects if some buttons
// are pressed, and switches some features around
// see the code further down for more details
static bool first_run = 1;
static bool fake_analog = 0;
static bool use_zl_for_l = 0;
static bool use_zr_for_r = 0;

uint8_t controller_state[20] = {0, 0};
uint8_t controller_state_bytes = 2;

static int snescontroller_poller(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;
  static uint32_t i;

  for(i=0; i<sizeof(switch_controller.bytes); i++) {
    switch_controller.bytes[i] = 0;
  }
  switch_controller.data.RX = 127;
  switch_controller.data.RY = 127;
  switch_controller.data.LX = 127;
  switch_controller.data.LY = 127;

  gpio_set(GPIOA, SNES_CLOCK);
  gpio_clear(GPIOA, SNES_LATCH);

  t = t+12;
  PT_WAIT_UNTIL(fibre_timeout(t));

  gpio_set(GPIOA, SNES_LATCH);
  t = t+12;
  PT_WAIT_UNTIL(fibre_timeout(t));
  gpio_clear(GPIOA, SNES_LATCH);
  t = t+12;
  PT_WAIT_UNTIL(fibre_timeout(t));

  for(i=0; i<16; i++) {
    gpio_toggle(GPIOA, SNES_CLOCK);
    t = t+12;
    PT_WAIT_UNTIL(fibre_timeout(t));

    if(gpio_get(GPIOA, SNES_DATA)) {
      _CLRBIT(controller_state[i/8], i % 8);
    } else {
      _SETBIT(controller_state[i/8], i % 8);
    }

    gpio_toggle(GPIOA, SNES_CLOCK);
    t = t+12;
    PT_WAIT_UNTIL(fibre_timeout(t));
  }

  switch_controller.data.A = _GETBIT(controller_state[1], 0);
  switch_controller.data.B = _GETBIT(controller_state[0], 0);
  switch_controller.data.X = _GETBIT(controller_state[1], 1);
  switch_controller.data.Y = _GETBIT(controller_state[0], 1);
  switch_controller.data.plus = _GETBIT(controller_state[0], 3);
  switch_controller.data.minus = _GETBIT(controller_state[0], 2);
  switch_controller.data.dpad_up = _GETBIT(controller_state[0], 4);
  switch_controller.data.dpad_down = _GETBIT(controller_state[0], 5);
  switch_controller.data.dpad_left = _GETBIT(controller_state[0], 6);
  switch_controller.data.dpad_right = _GETBIT(controller_state[0], 7);
  switch_controller.data.L = _GETBIT(controller_state[1], 2);
  switch_controller.data.R = _GETBIT(controller_state[1], 3);

  if(first_run) {
    if(switch_controller.data.A) fake_analog = 1;
    if(switch_controller.data.L) use_zl_for_l = 1;
    if(switch_controller.data.R) use_zr_for_r = 1;
  }
  first_run = 0;

  if(use_zl_for_l) {
    switch_controller.data.ZL = switch_controller.data.L;
    switch_controller.data.L = 0;
  }
  if(use_zr_for_r) {
    switch_controller.data.ZR = switch_controller.data.R;
    switch_controller.data.R = 0;
  }
  if(switch_controller.data.minus && switch_controller.data.plus) {
    switch_controller.data.home = 1;
    switch_controller.data.minus = 0;
    switch_controller.data.plus = 0;
  }

  if(fake_analog) {
    switch_controller.data.LX = 127 - (127 * switch_controller.data.dpad_left) + (128 * switch_controller.data.dpad_right);
    switch_controller.data.LY = 127 - (127 * switch_controller.data.dpad_up) + (128 * switch_controller.data.dpad_down);
    switch_controller.data.dpad_up = 0;
    switch_controller.data.dpad_down = 0;
    switch_controller.data.dpad_left = 0;
    switch_controller.data.dpad_right = 0;
  }

  PT_END();
}
static fibre_t snescontroller_poller_task = FIBRE_VAR_INIT(snescontroller_poller);

int poll_controller(void) {
  fibre_run_atomic(&snescontroller_poller_task);
  return 1;
}

void init_controller(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SNES_CLOCK);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SNES_LATCH);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, SNES_DATA);
}
