extern int usb_running;
void init_usb(void);
void usb_write(uint8_t *bytes, uint32_t len);
void usb_poll(void);
