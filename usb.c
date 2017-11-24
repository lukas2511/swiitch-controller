#include <stdlib.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "include/config.h"
#include "include/usb.h"

#ifdef INCLUDE_DFU_INTERFACE
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dfu.h>
#endif

static usbd_device *usbd_dev;
int usb_running = 0;

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

static const uint8_t hid_report_descriptor[] = {
  0x05, 0x01, /* USAGE_PAGE (Generic Desktop)         */
  0x09, 0x05, /* USAGE (Gamepad)                      */
  0xa1, 0x01, /* COLLECTION (Application)             */
  0x15, 0x00, /*     LOGICAL_MINIMUM (0)              */
  0x25, 0x01, /*     LOGICAL_MAXIMUM (1)              */
  0x35, 0x00, /*     PHYSICAL_MINIMUM (0)             */
  0x45, 0x01, /*     PHYSICAL_MAXIMUM (1)             */
  0x75, 0x01, /*     REPORT_SIZE (1)                  */
  0x95, 0x0e, /*     REPORT_COUNT (14)                */
  0x05, 0x09, /*     USAGE_PAGE (Buttons)             */
  0x19, 0x01, /*     USAGE_MINIMUM (Button 1)         */
  0x29, 0x0e, /*     USAGE_MAXIMUM (Button 14)        */
  0x81, 0x02, /*     INPUT (Data,Var,Abs)             */
  0x95, 0x02, /*     REPORT_COUNT (2)                 */
  0x81, 0x01, /*     INPUT (Data,Var,Abs)             */
  0x05, 0x01, /*     USAGE_PAGE (Generic Desktop Ctr) */
  0x25, 0x07, /*     LOGICAL_MAXIMUM (7)              */
  0x46, 0x3b, 0x01, /*     PHYSICAL_MAXIMUM (315)     */
  0x75, 0x04, /*     REPORT_SIZE (4)                  */
  0x95, 0x01, /*     REPORT_COUNT (1)                 */
  0x65, 0x14, /*     UNIT (20)                        */
  0x09, 0x39, /*     USAGE (Hat Switch)               */
  0x81, 0x42, /*     INPUT (Data,Var,Abs)             */
  0x65, 0x00, /*     UNIT (0)                         */
  0x95, 0x01, /*     REPORT_COUNT (1)                 */
  0x81, 0x01, /*     INPUT (Data,Var,Abs)             */
  0x26, 0xff, 0x00, /*     LOGICAL_MAXIMUM (255)      */
  0x46, 0xff, 0x00, /*     PHYSICAL_MAXIMUM (255)     */
  0x09, 0x30, /*     USAGE (Direction-X)              */
  0x09, 0x31, /*     USAGE (Direction-Y)              */
  0x09, 0x32, /*     USAGE (Direction-Z)              */
  0x09, 0x35, /*     USAGE (Rotate-Z)                 */
  0x75, 0x08, /*     REPORT_SIZE (8)                  */
  0x95, 0x04, /*     REPORT_COUNT (4)                 */
  0x81, 0x02, /*     INPUT (Data,Var,Abs)             */
  0x75, 0x08, /*     REPORT_SIZE (8)                  */
  0x95, 0x01, /*     REPORT_COUNT (1)                 */
  0x81, 0x01, /*     INPUT (Data,Var,Abs)             */
  0xc0,       /*   END_COLLECTION                     */
};

static const struct {
  struct usb_hid_descriptor hid_descriptor;
  struct {
    uint8_t bReportDescriptorType;
    uint16_t wDescriptorLength;
  } __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
  .hid_descriptor = {
    .bLength = sizeof(hid_function),
    .bDescriptorType = USB_DT_HID,
    .bcdHID = 0x0111,
    .bCountryCode = 0,
    .bNumDescriptors = 1,
  },
  .hid_report = {
    .bReportDescriptorType = USB_DT_REPORT,
    .wDescriptorLength = sizeof(hid_report_descriptor),
  }
};

const struct usb_endpoint_descriptor hid_endpoint[] = {{
  .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x02,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 64,
    .bInterval = 5,
}, {
  .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = USB_DT_ENDPOINT,
    .bEndpointAddress = 0x81,
    .bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
    .wMaxPacketSize = 64,
    .bInterval = 5,
}};

const struct usb_interface_descriptor hid_iface = {
  .bLength = USB_DT_INTERFACE_SIZE,
  .bDescriptorType = USB_DT_INTERFACE,
  .bInterfaceNumber = 0,
  .bAlternateSetting = 0,
  .bNumEndpoints = 2,
  .bInterfaceClass = 3, // HID
  .bInterfaceSubClass = 0,
  .bInterfaceProtocol = 0,
  .iInterface = 0,

  .endpoint = hid_endpoint,

  .extra = &hid_function,
  .extralen = sizeof(hid_function),
};

#ifdef INCLUDE_DFU_INTERFACE
const struct usb_dfu_descriptor dfu_function = {
  .bLength = sizeof(struct usb_dfu_descriptor),
  .bDescriptorType = DFU_FUNCTIONAL,
  .bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_WILL_DETACH,
  .wDetachTimeout = 255,
  .wTransferSize = 1024,
  .bcdDFUVersion = 0x011A,
};

const struct usb_interface_descriptor dfu_iface = {
  .bLength = USB_DT_INTERFACE_SIZE,
  .bDescriptorType = USB_DT_INTERFACE,
  .bInterfaceNumber = 1,
  .bAlternateSetting = 0,
  .bNumEndpoints = 0,
  .bInterfaceClass = 0xFE,
  .bInterfaceSubClass = 1,
  .bInterfaceProtocol = 1,
  .iInterface = 0,

  .extra = &dfu_function,
  .extralen = sizeof(dfu_function),
};
#endif

const struct usb_interface ifaces[] = {{
  .num_altsetting = 1,
    .altsetting = &hid_iface,
#ifdef INCLUDE_DFU_INTERFACE
}, {
  .num_altsetting = 1,
    .altsetting = &dfu_iface,
#endif
}};

const struct usb_config_descriptor config = {
  .bLength = USB_DT_CONFIGURATION_SIZE,
  .bDescriptorType = USB_DT_CONFIGURATION,
  .wTotalLength = 0,
#ifdef INCLUDE_DFU_INTERFACE
  .bNumInterfaces = 2,
#else
  .bNumInterfaces = 1,
#endif
  .bConfigurationValue = 1,
  .iConfiguration = 0,
  .bmAttributes = 0x80,
  .bMaxPower = 0xFA,

  .interface = ifaces,
};

static const char *usb_strings[] = {
  "HORI CO.,LTD.",
  "HORIPAD S",
  "",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static int hid_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *))
{
  (void)complete;
  (void)dev;

  if((req->bmRequestType != 0x81) ||
      (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
      (req->wValue != 0x2200))
    return 0;

  /* Handle the HID report descriptor. */
  *buf = (uint8_t *)hid_report_descriptor;
  *len = sizeof(hid_report_descriptor);

  return 1;
}

#ifdef INCLUDE_DFU_INTERFACE
static void dfu_detach_complete(usbd_device *dev, struct usb_setup_data *req)
{
  (void)req;
  (void)dev;

  char *const marker = (char *)0x20004800;
  const char key[] = "remain-in-loader";
  memcpy(marker, key, sizeof(key));
  scb_reset_system();
}

static int dfu_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
    void (**complete)(usbd_device *, struct usb_setup_data *))
{
  (void)buf;
  (void)len;
  (void)dev;

  if ((req->bmRequestType != 0x21) || (req->bRequest != DFU_DETACH))
    return 0; /* Only accept class request. */

  *complete = dfu_detach_complete;

  return 1;
}
#endif

static void hid_set_config(usbd_device *dev, uint16_t wValue)
{
  (void)wValue;
  (void)dev;

  usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);

  usbd_register_control_callback(
      dev,
      USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
      USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
      hid_control_request);
#ifdef INCLUDE_DFU_INTERFACE
  usbd_register_control_callback(
      dev,
      USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
      USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
      dfu_control_request);
#endif

  usb_running = 1;
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

  usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
  usbd_register_set_config_callback(usbd_dev, hid_set_config);
}

void usb_poll(void) {
  usbd_poll(usbd_dev);
}

void usb_write(uint8_t *bytes, uint32_t len) {
  usbd_ep_write_packet(usbd_dev, 0x81, bytes, len);
}
