#include <stdlib.h>
//#include <librfn/fibre.h>
#include <string.h>
#include <librfn/console.h>
#include "include/myconsole.h"
#include <libopencm3/cm3/scb.h>
#include <librfn/time.h>

console_t console;

void console_hwinit(console_t *c) {
  console_silent(c);
}

static pt_state_t console_uptime(console_t *c) {
  if (c->argc != 1)
    fprintf(c->out, "Usage: uptime\n");
  else
    fprintf(c->out, "I've been running for %ld ms\n", time_now() / 1000l);

  return PT_EXITED;
}
static const console_cmd_t cmd_uptime = CONSOLE_CMD_VAR_INIT("uptime", console_uptime);

static pt_state_t console_bootloader(console_t *c) {
  (void)c;
  char *const marker = (char *)0x20004800;
  const char key[] = "remain-in-loader";
  memcpy(marker, key, sizeof(key));
  scb_reset_system();
  return PT_EXITED;
}
static const console_cmd_t cmd_bootloader = CONSOLE_CMD_VAR_INIT("bootloader", console_bootloader);


void init_myconsole(void) {
  console_init(&console, stdout);
  console_register(&cmd_uptime);
  console_register(&cmd_bootloader);
}
