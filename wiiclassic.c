#include <stdlib.h>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <librfm3/i2c_ctx.h>

#include "wiiclassic.h"

static void i2c_init(void) {
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_AFIO);

	i2c_ctx_t ctx;
	i2c_ctx_init(&ctx, I2C1);
	i2c_ctx_reset(&ctx);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO6 | GPIO7);

	// TODO

}

void init_wiiclassic(void) {
	i2c_init();
}
