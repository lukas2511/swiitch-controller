#include <libopencm3/usb/usbd.h>

#define MAX_USB_IFACES 10
#define MAX_USB_ENDPOINTS 10

extern usbd_device *usbd_dev;
extern int usb_running;
void init_usb(void);
void usb_write(uint8_t *bytes, uint32_t len);

uint8_t register_usb_interface_descriptor(struct usb_interface_descriptor *iface_descr);
void register_usb_endpoint(uint8_t addr, uint8_t type, uint16_t max_size, usbd_endpoint_callback callback);
void register_usb_callback(uint8_t type, uint8_t type_mask, usbd_control_callback callback);
