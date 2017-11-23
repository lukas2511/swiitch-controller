#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>

#include "controller.h"
#include "usb.h"

int main(void)
{
  rcc_clock_setup_in_hsi_out_48mhz();

  rcc_periph_clock_enable(RCC_GPIOA);
  /*
   * This is a somewhat common cheap hack to trigger device re-enumeration
   * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
   * setting the pin to output, and driving it explicitly low effectively
   * "removes" the pullup.  The subsequent USB init will "take over" the
   * pin, and it will appear as a proper pullup to the host.
   * The magic delay is somewhat arbitrary, no guarantees on USBIF
   * compliance here, but "it works" in most places.
   */
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
  gpio_clear(GPIOA, GPIO12);
  for (unsigned i = 0; i < 800000; i++) {
    __asm__("nop");
  }

  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
  /* SysTick interrupt every N clock pulses: set reload to N-1 */
  systick_set_reload(99999);
  systick_interrupt_enable();
  systick_counter_enable();

  init_usb();
}

union Controller controller;

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
