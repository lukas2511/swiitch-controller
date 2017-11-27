#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <librfn/fibre.h>
#include <librfn/console.h>
#include "include/myconsole.h"
#include <librfn/time.h>

#include "include/controllers.h"
#include "include/usb.h"
#include "include/wiiclassic.h"

union SwitchController switch_controller;

static int main_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;
  static uint8_t size = 0;

  for(uint32_t i=0; i<sizeof(switch_controller.bytes); i++) {
    switch_controller.bytes[i] = 0;
  }

  while(1) {
    // 1ms timeout
    t = time_now() + 1000;
    PT_WAIT_UNTIL(fibre_timeout(t));
    gpio_toggle(GPIOC, GPIO13);

    if(usb_running) {
      //size = poll_wiiclassic();
      if(size == 0) continue;

      // LT and RT are unused
      switch_controller.data.RX = (wiiclassic_controller.data.RX_4_3 << 3 | wiiclassic_controller.data.RX_2_1 << 1 | wiiclassic_controller.data.RX_0) << 3;
      switch_controller.data.RY = wiiclassic_controller.data.RY << 3;
      switch_controller.data.LX = wiiclassic_controller.data.LX << 2;
      switch_controller.data.LY = wiiclassic_controller.data.LY << 2;
      switch_controller.data.A = wiiclassic_controller.data.A;
      switch_controller.data.B = wiiclassic_controller.data.B;
      switch_controller.data.X = wiiclassic_controller.data.X;
      switch_controller.data.Y = wiiclassic_controller.data.Y;
      switch_controller.data.plus = wiiclassic_controller.data.plus;
      switch_controller.data.minus = wiiclassic_controller.data.minus;
      switch_controller.data.home = wiiclassic_controller.data.home;
      switch_controller.data.dpad_up = wiiclassic_controller.data.dpad_up;
      switch_controller.data.dpad_down = wiiclassic_controller.data.dpad_down;
      switch_controller.data.dpad_left = wiiclassic_controller.data.dpad_left;
      switch_controller.data.dpad_right = wiiclassic_controller.data.dpad_right;
      switch_controller.data.L = wiiclassic_controller.data.L;
      switch_controller.data.R = wiiclassic_controller.data.R;
      switch_controller.data.ZL = wiiclassic_controller.data.ZL;
      switch_controller.data.ZR = wiiclassic_controller.data.ZR;

      usb_write(switch_controller.bytes, 8);
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
