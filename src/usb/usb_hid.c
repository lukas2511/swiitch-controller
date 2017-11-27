#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "config.h"
#include "usb.h"
#include "usb_hid.h"

#include <librfn/fibre.h>
#include <librfn/time.h>

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

static struct usb_interface_descriptor hid_iface = {
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

void init_usb_hid(void) {
  register_usb_interface_descriptor(&hid_iface);
  register_usb_endpoint(0x81, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);
  register_usb_callback(USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, hid_control_request);
}

void usb_hid_write(uint8_t *bytes, uint32_t len) {
  usbd_ep_write_packet(usbd_dev, 0x81, bytes, len);
}
