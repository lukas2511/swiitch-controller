#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

#include "controller.h"
#include "usb.h"

union Controller controller;

int main(void)
{
  rcc_clock_setup_in_hsi_out_48mhz();

  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
  /* SysTick interrupt every N clock pulses: set reload to N-1 */
  systick_set_reload(99999);
  systick_interrupt_enable();
  systick_counter_enable();

  init_usb();
}

void sys_tick_handler(void)
{
  for(uint32_t i=0; i<sizeof(controller.bytes); i++) {
    controller.bytes[i] = 0;
  }

  // TODO: insert appropiate code here
  controller.data.LX = 127;
  controller.data.LY = 127;
  controller.data.RX = 127;
  controller.data.RY = 127;

  if(usb_running) {
    usb_write(controller.bytes);
  }
}
