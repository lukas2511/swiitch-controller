#include <stdlib.h>

#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <librfm3/i2c_ctx.h>

#include <librfn/console.h>
#include "include/myconsole.h"
#include "include/i2c.h"

void i2c_write(uint8_t addr, uint8_t* buf, int len) {
	i2c_ctx_t ctx;
	i2c_ctx_init(&ctx, I2C1);
	PT_CALL(&ctx.leaf, i2c_ctx_start(&ctx));
	if(ctx.err) goto err;
	PT_CALL(&ctx.leaf, i2c_ctx_sendaddr(&ctx, addr, 0));
	if(ctx.err) goto err;

	for(int i=0;i<len;i++) {
		PT_CALL(&ctx.leaf, i2c_ctx_senddata(&ctx, buf[i]));
		if(ctx.err) goto err;
	}

	PT_CALL(&ctx.leaf, i2c_ctx_stop(&ctx));
	if(ctx.err) goto err;

err:
	i2c_ctx_reset(&ctx);
}

int i2c_read(uint8_t addr, uint8_t* buf, int len) {
	i2c_ctx_t ctx;
	i2c_ctx_init(&ctx, I2C1);
	PT_CALL(&ctx.leaf, i2c_ctx_start(&ctx));
	if(ctx.err) goto err;
	PT_CALL(&ctx.leaf, i2c_ctx_sendaddr(&ctx, addr, len));
	if(ctx.err) goto err;

	for(int i=0;i<len;i++) {
		PT_CALL(&ctx.leaf, i2c_ctx_getdata(&ctx, buf + i));
		if(ctx.err) goto err;
	}

	return len;

err:
	i2c_ctx_reset(&ctx);
	return -1;
}

static int parse_integer(char* bytes) {
	if(bytes[0] == '0' && bytes[1] == 'x') {
		int out = 0;

		if(bytes[2] <= '9') out = out + 16 * (bytes[2] - 48);
		else out = out + 16 * (bytes[2] - 97 + 10);

		if(bytes[3] <= '9') out = out + (bytes[3] - 48);
		else out = out + (bytes[3] - 97 + 10);

		return out;
	} else {
		return atoi(bytes);
	}
}

static pt_state_t console_i2c_read(console_t *c)
{
	if (c->argc != 3) {
		fprintf(c->out, "Usage: i2c_read <addr> <length>\n");
	} else {
		uint8_t addr = parse_integer(c->argv[1]);
		int len = parse_integer(c->argv[2]);
		uint8_t buf[len];
		fprintf(c->out, "Reading %d bytes from 0x%02x...\n", len, addr);

		i2c_read(addr, buf, len);
		for(int i=0;i<len;i++) {
			fprintf(c->out, "%02x ", buf[i]);
		}
		fprintf(c->out, "\r\n");
	}

	return PT_EXITED;
}
static const console_cmd_t cmd_i2c_read = CONSOLE_CMD_VAR_INIT("i2c_read", console_i2c_read);

static pt_state_t console_i2c_write(console_t *c)
{
	if (c->argc < 3) {
		fprintf(c->out, "Usage: i2c_write <addr> <byte1> <byte2> <byte3>\n");
	} else {
		uint8_t addr = parse_integer(c->argv[1]);
		int len = c->argc - 2;
		uint8_t buf[len];
		for(int i=0;i<len;i++) {
			buf[i] = parse_integer(c->argv[i + 2]);
		}
		fprintf(c->out, "Sending %d bytes to 0x%02x...\n", len, addr);
		i2c_write(addr, buf, len);
		fprintf(c->out, "Done!\n");
	}

	return PT_EXITED;
}
static const console_cmd_t cmd_i2c_write = CONSOLE_CMD_VAR_INIT("i2c_write", console_i2c_write);

void init_i2c(void) {
	/* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Enable clocks for I2C1 and AFIO. */
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_AFIO);

	/* initialize the peripheral */
	i2c_ctx_t ctx;
	i2c_ctx_init(&ctx, I2C1);
	i2c_ctx_reset(&ctx);

	/* GPIO for I2C1 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN, GPIO6 | GPIO7);

	console_register(&cmd_i2c_read);
	console_register(&cmd_i2c_write);
}
