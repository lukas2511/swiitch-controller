#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <librfn/fibre.h>
#include <librfn/time.h>

#include "controller.h"
#include "usb.h"
#include "wiiclassic.h"

union Controller controller;

static int usb_fibre(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  while(1) {
    usb_poll();
    PT_YIELD();
  }
  PT_END();
}
static fibre_t usb_task = FIBRE_VAR_INIT(usb_fibre);

static int x = 64;

static int main_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;
  while(1) {
    gpio_toggle(GPIOC, GPIO13);

    if(usb_running) {
      for(uint32_t i=0; i<sizeof(controller.bytes); i++) {
        controller.bytes[i] = 0;
      }
      // TODO: get controller state

      controller.data.A = 1;
      controller.data.LX = x;
      x++;
      if(x==255) x=0;
      controller.data.LY = 127;
      controller.data.RX = 127;
      controller.data.RY = 127;

      usb_write(controller.bytes);
    }

    // 1ms timeout
    t = time_now() + 1000;
    PT_WAIT_UNTIL(fibre_timeout(t));
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

  fibre_run(&usb_task);
  fibre_run(&main_loop_task);
  fibre_scheduler_main_loop();
}
