#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <librfn/fibre.h>
#include <librfn/console.h>
#include "include/myconsole.h"
#include <librfn/time.h>

#include "include/controller.h"
#include "usb.h"
#include "usb_cdc.h"
#include "usb_hid.h"
#include "usb_dfu.h"

union SwitchController switch_controller;

static int main_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;

  for(uint32_t i=0; i<sizeof(switch_controller.bytes); i++) {
    switch_controller.bytes[i] = 0;
  }

  while(1) {
    // 100us timeout
    t = time_now() + 100;
    PT_WAIT_UNTIL(fibre_timeout(t));

    if(usb_running) {
      if(poll_controller()) {
        // DPAD
        switch(switch_controller.bytes[8]) {
          case 0x80: switch_controller.data.HAT = 0x00; break;
          case 0xA0: switch_controller.data.HAT = 0x01; break;
          case 0x20: switch_controller.data.HAT = 0x02; break;
          case 0x60: switch_controller.data.HAT = 0x03; break;
          case 0x40: switch_controller.data.HAT = 0x04; break;
          case 0x50: switch_controller.data.HAT = 0x05; break;
          case 0x10: switch_controller.data.HAT = 0x06; break;
          case 0x90: switch_controller.data.HAT = 0x07; break;
          default: switch_controller.data.HAT = 0x08; break;
        }

        // Send data
        usb_hid_write(switch_controller.bytes, 8);
      }
    }
  }
  PT_END();
}
static fibre_t main_loop_task = FIBRE_VAR_INIT(main_loop);

static int debug_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;

  while(1) {
    // 500ms timeout
    t = time_now() + 500000;
    PT_WAIT_UNTIL(fibre_timeout(t));
    gpio_toggle(GPIOC, GPIO13);
  }
  PT_END();
}
static fibre_t debug_loop_task = FIBRE_VAR_INIT(debug_loop);


int main(void) {
  rcc_clock_setup_in_hse_8mhz_out_72mhz();
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  time_init();
  init_myconsole();

  init_usb();
  init_usb_hid();
  init_usb_dfu();
  init_usb_cdc();

  init_controller();

  fibre_run(&main_loop_task);
  fibre_run(&debug_loop_task);
  fibre_scheduler_main_loop();
}
