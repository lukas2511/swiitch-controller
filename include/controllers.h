union SwitchController {
  struct {
    // buttons
    unsigned int Y : 1;
    unsigned int B : 1;
    unsigned int A : 1;
    unsigned int X : 1;
    unsigned int L : 1;
    unsigned int R : 1;
    unsigned int ZL : 1;
    unsigned int ZR : 1;
    unsigned int minus : 1;
    unsigned int plus : 1;
    unsigned int lclick : 1;
    unsigned int rclick : 1;
    unsigned int home : 1;
    unsigned int capture : 1;
    unsigned int : 2; // unused bits
    unsigned int dpad_up : 1;
    unsigned int dpad_down : 1;
    unsigned int dpad_right : 1;
    unsigned int dpad_left : 1;
    unsigned int : 4; // unused bits
    // other stuff
    uint8_t LX : 8;
    uint8_t LY : 8;
    uint8_t RX : 8;
    uint8_t RY : 8;
    unsigned int : 8; // vendor specific
  } data;
  uint8_t bytes[8];
};

