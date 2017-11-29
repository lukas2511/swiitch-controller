#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "usb.h"
#include "usb_cdc.h"

#include <librfn/fibre.h>
#include <librfn/time.h>

usbd_device *usbd_dev;
int usb_running = 0;

struct registered_endpoint {
  uint8_t addr;
  uint8_t type;
  uint16_t max_size;
  usbd_endpoint_callback callback;
};
static struct registered_endpoint registered_endpoints[MAX_USB_ENDPOINTS];
static int num_registered_endpoints = 0;

struct registered_callback {
  uint8_t type;
  uint8_t type_mask;
  usbd_control_callback callback;
};
static struct registered_callback registered_callbacks[MAX_USB_ENDPOINTS];
static int num_registered_callbacks;

const struct usb_device_descriptor dev_descr = {
  .bLength = USB_DT_DEVICE_SIZE,
  .bDescriptorType = USB_DT_DEVICE,
  .bcdUSB = 0x0200,
  .bDeviceClass = 0,
  .bDeviceSubClass = 0,
  .bDeviceProtocol = 0,
  .bMaxPacketSize0 = 64,
  .idVendor = 0x0f0d,
  .idProduct = 0x00c1,
  .bcdDevice = 0x0572,
  .iManufacturer = 1,
  .iProduct = 2,
  .iSerialNumber = 0,
  .bNumConfigurations = 1,
};

struct usb_interface usb_ifaces[MAX_USB_IFACES] = {{}, {}, {}, {}};

struct usb_config_descriptor usb_config = {
  .bLength = USB_DT_CONFIGURATION_SIZE,
  .bDescriptorType = USB_DT_CONFIGURATION,
  .wTotalLength = 0,
  .bNumInterfaces = 0,
  .bConfigurationValue = 1,
  .iConfiguration = 0,
  .bmAttributes = 0x80,
  .bMaxPower = 0xFA,

  .interface = usb_ifaces,
};

static const char *usb_strings[] = {
  "LUKAS2511",
  "FRICKELPAD S",
  "",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static void set_usb_config(usbd_device *dev, uint16_t wValue)
{
  (void)wValue;
  (void)dev;

  for(int i=0; i<num_registered_endpoints; i++) {
    usbd_ep_setup(dev, registered_endpoints[i].addr, registered_endpoints[i].type, registered_endpoints[i].max_size, registered_endpoints[i].callback);
  }

  for(int i=0; i<num_registered_callbacks; i++) {
    usbd_register_control_callback(dev, registered_callbacks[i].type, registered_callbacks[i].type_mask, registered_callbacks[i].callback);
  }

  usb_running = 1;
}

static int usb_fibre(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  while(1) {
    usbd_poll(usbd_dev);
    PT_YIELD();
  }
  PT_END();
}
static fibre_t usb_task = FIBRE_VAR_INIT(usb_fibre);

uint8_t register_usb_interface_descriptor(struct usb_interface_descriptor *iface_descr) {
  if(usb_config.bNumInterfaces < MAX_USB_IFACES) {
    iface_descr->bInterfaceNumber = usb_config.bNumInterfaces;
    usb_ifaces[usb_config.bNumInterfaces].num_altsetting = 1;
    usb_ifaces[usb_config.bNumInterfaces].altsetting = iface_descr;
    return usb_config.bNumInterfaces++;
  }
  return -1;
}

void register_usb_endpoint(uint8_t addr, uint8_t type, uint16_t max_size, usbd_endpoint_callback callback) {
  const struct registered_endpoint new_endpoint = {
    .addr = addr,
    .type = type,
    .max_size = max_size,
    .callback = callback,
  };
  registered_endpoints[num_registered_endpoints++] = new_endpoint;
}

void register_usb_callback(uint8_t type, uint8_t type_mask, usbd_control_callback callback) {
  const struct registered_callback new_callback = {
    .type = type,
    .type_mask = type_mask,
    .callback = callback,
  };
  registered_callbacks[num_registered_callbacks++] = new_callback;
}

void init_usb(void) {
  rcc_periph_clock_enable(RCC_GPIOA);
  /*
   * This is a somewhat common cheap hack to trigger device re-enumeration
   * on startup.  Assuming a fixed external pullup on D+, (For USB-FS)
   * setting the pin to output, and driving it explicitly low effectively
   * "removes" the pullup.  The subsequent USB init will "take over" the
   * pin, and it will appear as a proper pullup to the host.
   * The magic delay is somewhat arbitrary, no guarantees on USBIF
   * compliance here, but "it works" in most places.
   */
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
  gpio_clear(GPIOA, GPIO12);
  for (unsigned i = 0; i < 800000; i++) {
    __asm__("nop");
  }

  usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &usb_config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
  usbd_register_set_config_callback(usbd_dev, set_usb_config);

  fibre_run(&usb_task);
}
