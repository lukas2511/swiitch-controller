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

union WiiClassicController {
  struct {
    // byte1
    unsigned int RX_4_3 : 2; // what
    unsigned int LX : 6;
    // byte2
    unsigned int RX_2_1 : 2; // the
    unsigned int LY : 6;
    // byte3
    unsigned int RX_0 : 1; // actual
    unsigned int LT_4_3 : 2; // fuck
    unsigned int RY : 5;
    // byte4
    unsigned int LT_2_0 : 3; // nintendo?!?!
    unsigned int RT : 5;
    // byte5
    unsigned int dpad_right : 1;
    unsigned int dpad_down : 1;
    unsigned int L : 1;
    unsigned int minus : 1;
    unsigned int home : 1;
    unsigned int plus : 1;
    unsigned int R : 1;
    unsigned int : 1; // unused / always 1 ?
    // byte6
    unsigned int ZL : 1;
    unsigned int B : 1;
    unsigned int Y : 1;
    unsigned int A : 1;
    unsigned int X : 1;
    unsigned int ZR : 1;
    unsigned int dpad_left : 1;
    unsigned int dpad_up : 1;
  } data;
  uint8_t bytes[6];
};
