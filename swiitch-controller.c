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
#include "include/usb.h"
#include "include/wiiclassic.h"

union SwitchController switch_controller;

static int main_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;

  for(uint32_t i=0; i<sizeof(switch_controller.bytes); i++) {
    switch_controller.bytes[i] = 0;
  }

  while(1) {
    // 1ms timeout
    t = time_now() + 1000;
    PT_WAIT_UNTIL(fibre_timeout(t));
    gpio_toggle(GPIOC, GPIO13);

    if(usb_running) {
      if(poll_wiiclassic()) {
        usb_write(switch_controller.bytes, 8);
      }
    }
  }
  PT_END();
}
static fibre_t main_loop_task = FIBRE_VAR_INIT(main_loop);

int main(void) {
  rcc_clock_setup_in_hsi_out_48mhz();
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  time_init();

  init_usb();
  init_wiiclassic();
  init_myconsole();

  fibre_run(&main_loop_task);
  fibre_scheduler_main_loop();
}
