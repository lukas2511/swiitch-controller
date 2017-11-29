union SwitchController {
  struct {
    // byte1
    unsigned int Y : 1;
    unsigned int B : 1;
    unsigned int A : 1;
    unsigned int X : 1;
    unsigned int L : 1;
    unsigned int R : 1;
    unsigned int ZL : 1;
    unsigned int ZR : 1;
    // byte2
    unsigned int minus : 1;
    unsigned int plus : 1;
    unsigned int lclick : 1;
    unsigned int rclick : 1;
    unsigned int home : 1;
    unsigned int capture : 1;
    unsigned int : 2; // unused bits
    // byte3
    unsigned int HAT : 4;
    unsigned int : 4; // unused bits
    // byte4
    uint8_t LX : 8;
    // byte5
    uint8_t LY : 8;
    // byte6
    uint8_t RX : 8;
    // byte7
    uint8_t RY : 8;
    // byte8
    unsigned int : 8; // vendor specific

    // custom/temp data
    unsigned int : 4;
    unsigned int dpad_left : 1;
    unsigned int dpad_right : 1;
    unsigned int dpad_down : 1;
    unsigned int dpad_up : 1;
  } data;
  uint8_t bytes[9];
};
extern union SwitchController switch_controller;

extern uint8_t controller_state[20];
extern uint8_t controller_state_bytes;
void init_controller(void);
int poll_controller(void);
