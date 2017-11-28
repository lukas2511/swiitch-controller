#include <stdlib.h>
#include <string.h>
#include <librfn/console.h>
#include "myconsole.h"
#include "controller.h"
#include <libopencm3/cm3/scb.h>
#include <librfn/time.h>
#include "usb_cdc.h"

#define CLEAR_SCREEN fprintf(console.out, "\e[1;1H\e[2J");
#define BOLD fprintf(console.out, "\033[1m");
#define RESET fprintf(console.out, "\033[0m");
#define RED fprintf(console.out, "\033[31;1m");
#define MOVE_CURSOR(l, c) fprintf(console.out, "\033[%d;%dH", l, c);
#define GREEN fprintf(console.out, "\033[32;1m");

console_t console;

void console_hwinit(console_t *c) {
  console_silent(c);
}

// uptime
static pt_state_t console_uptime(console_t *c) {
  if (c->argc != 1)
    fprintf(c->out, "Usage: uptime\n");
  else
    fprintf(c->out, "I've been running for %ld ms\n", time_now() / 1000l);

  return PT_EXITED;
}
static const console_cmd_t cmd_uptime = CONSOLE_CMD_VAR_INIT("uptime", console_uptime);

// bootloader
static pt_state_t console_bootloader(console_t *c) {
  (void)c;
  char *const marker = (char *)0x20004800;
  const char key[] = "remain-in-loader";
  memcpy(marker, key, sizeof(key));
  scb_reset_system();
  return PT_EXITED;
}
static const console_cmd_t cmd_bootloader = CONSOLE_CMD_VAR_INIT("bootloader", console_bootloader);

// watcher
static int print_loop_countdown = 0;
static void print_controller_state(void) {
  MOVE_CURSOR(2, 2)
    fprintf(console.out, "Uptime: %ldms", time_now() / 1000);

  MOVE_CURSOR(4, 2)
    BOLD
    fprintf(console.out, "Simulated Output");

  // left analog stick
  MOVE_CURSOR(2+7, 4)
    fprintf(console.out, "LX: %d  ", switch_controller.data.LX);
  MOVE_CURSOR(2+8, 4)
    fprintf(console.out, "LY: %d  ", switch_controller.data.LY);

  // buttons dpad
  MOVE_CURSOR(7+7, 6)
    if(switch_controller.data.dpad_up) GREEN else RED
      fprintf(console.out, "u");
  MOVE_CURSOR(7+8, 4)
    if(switch_controller.data.dpad_left) GREEN else RED
      fprintf(console.out, "l");
  MOVE_CURSOR(7+8, 8)
    if(switch_controller.data.dpad_right) GREEN else RED
      fprintf(console.out, "r");
  MOVE_CURSOR(7+9, 6)
    if(switch_controller.data.dpad_down) GREEN else RED
      fprintf(console.out, "d");

  // buttons Z/ZL/R/ZR
  MOVE_CURSOR(6, 3)
    if(switch_controller.data.ZL) GREEN else RED
      fprintf(console.out, "ZL");
  MOVE_CURSOR(6, 6)
    if(switch_controller.data.L) GREEN else RED
      fprintf(console.out, "L");
  MOVE_CURSOR(6, 36)
    if(switch_controller.data.R) GREEN else RED
      fprintf(console.out, "R");
  MOVE_CURSOR(6, 39)
    if(switch_controller.data.ZR) GREEN else RED
      fprintf(console.out, "ZR");

  // buttons +,H,-,C
  MOVE_CURSOR(11, 19)
    if(switch_controller.data.minus) GREEN else RED
      fprintf(console.out, "-");
  MOVE_CURSOR(11, 21)
    if(switch_controller.data.home) GREEN else RED
      fprintf(console.out, "H");
  MOVE_CURSOR(12, 21)
    if(switch_controller.data.capture) GREEN else RED
      fprintf(console.out, "C");
  MOVE_CURSOR(11, 23)
    if(switch_controller.data.plus) GREEN else RED
      fprintf(console.out, "+");

  // buttons A,B,X,Y
  MOVE_CURSOR(2+7, 30+6)
    if(switch_controller.data.X) GREEN else RED
      fprintf(console.out, "X");
  MOVE_CURSOR(2+8, 30+4)
    if(switch_controller.data.Y) GREEN else RED
      fprintf(console.out, "Y");
  MOVE_CURSOR(2+8, 30+8)
    if(switch_controller.data.A) GREEN else RED
      fprintf(console.out, "A");
  MOVE_CURSOR(2+9, 30+6)
    if(switch_controller.data.B) GREEN else RED
      fprintf(console.out, "B");
  RESET

  BOLD
  // right analog stick
  MOVE_CURSOR(15, 30+3)
    fprintf(console.out, "RX: %d  ", switch_controller.data.RX);
  MOVE_CURSOR(16, 30+3)
    fprintf(console.out, "RY: %d  ", switch_controller.data.RY);

  MOVE_CURSOR(18, 2)
    fprintf(console.out, "Raw Input: ");
    for(uint8_t i=0; i<controller_state_bytes; i++) {
      fprintf(console.out, "0x%02x ", controller_state[i]);
    }

  RESET
  MOVE_CURSOR(19, 0)

  fflush(console.out);
}
static bool watch_controller_state = 0;
static int watcher_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;

  while(1) {
    // 100ms timeout
    t = time_now() + 100000;
    PT_WAIT_UNTIL(fibre_timeout(t));
    if (watch_controller_state) {
      if(print_loop_countdown==0) {
        print_loop_countdown = 100;
        CLEAR_SCREEN
      }
      print_loop_countdown--;

      print_controller_state();
    }
  }
  PT_END();
}
static fibre_t watcher_loop_task = FIBRE_VAR_INIT(watcher_loop);

static pt_state_t console_state(console_t *c) {
  (void)c;
  if(c->argc==2 && strncmp(c->argv[1], "watch", 5)==0) {
    watch_controller_state = 1;
  } else {
    watch_controller_state = 0;
    CLEAR_SCREEN
    print_controller_state();
  }
  return PT_EXITED;
}
static const console_cmd_t cmd_state = CONSOLE_CMD_VAR_INIT("state", console_state);


void init_myconsole(void) {
  console_init(&console, stdout);
  console_register(&cmd_uptime);
  console_register(&cmd_bootloader);
  console_register(&cmd_state);
  fibre_run(&watcher_loop_task);
}
