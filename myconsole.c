#include <stdlib.h>
//#include <librfn/fibre.h>
#include <librfn/console.h>
#include "include/myconsole.h"
#include <librfn/time.h>

console_t console;

void console_hwinit(console_t *c) {
  console_silent(c);
}

static pt_state_t console_uptime(console_t *c)
{
  if (c->argc != 1)
    fprintf(c->out, "Usage: uptime\n");
  else
    fprintf(c->out, "I've been running for %ld ms\n", time_now() / 1000l);

  return PT_EXITED;
}
static const console_cmd_t cmd_add =
    CONSOLE_CMD_VAR_INIT("uptime", console_uptime);


void init_myconsole(void) {
  console_init(&console, stdout);
  console_register(&cmd_add);
}
