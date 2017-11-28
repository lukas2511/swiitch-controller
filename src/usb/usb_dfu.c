#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "usb.h"
#include "usb_dfu.h"

#include <librfn/fibre.h>
#include <librfn/time.h>

#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dfu.h>

const struct usb_dfu_descriptor dfu_function = {
  .bLength = sizeof(struct usb_dfu_descriptor),
  .bDescriptorType = DFU_FUNCTIONAL,
  .bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
  .wDetachTimeout = 255,
  .wTransferSize = 1024,
  .bcdDFUVersion = 0x011A,
};

static struct usb_interface_descriptor dfu_iface = {
  .bLength = USB_DT_INTERFACE_SIZE,
  .bDescriptorType = USB_DT_INTERFACE,
  .bInterfaceNumber = 0,
  .bAlternateSetting = 0,
  .bNumEndpoints = 0,
  .bInterfaceClass = 0xFE,
  .bInterfaceSubClass = 1,
  .bInterfaceProtocol = 1,
  .iInterface = 0,

  .extra = &dfu_function,
  .extralen = sizeof(dfu_function),
};

static int dfu_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *))
{
  (void)complete;
  (void)buf;
  (void)len;
  (void)dev;

  if ((req->bmRequestType != DFU_FUNCTIONAL) || (req->bRequest != DFU_DETACH))
    return 0; /* Only accept class request. */

  printf("Resetting!\n");

  char *const marker = (char *)0x20004800;
  const char key[] = "remain-in-loader";
  memcpy(marker, key, sizeof(key));
  scb_reset_system();

  return 1;
}

void init_usb_dfu(void) {
  register_usb_interface_descriptor(&dfu_iface);
  register_usb_callback(USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, dfu_control_request);
}
