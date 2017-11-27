#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "include/config.h"
#include "include/usb.h"
#include <librfn/fibre.h>
#include <librfn/time.h>

#ifdef INCLUDE_DFU_INTERFACE
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/dfu.h>
#endif

#ifdef INCLUDE_CDC_INTERFACE
#include <librfn/util.h>
#include <librfn/console.h>
#include <librfn/ringbuf.h>
#include <libopencm3/usb/cdc.h>
#include "include/myconsole.h"
int _write(int file, char *ptr, int len);

static uint8_t outbuf[1024];
static ringbuf_t outring = RINGBUF_VAR_INIT(outbuf, sizeof(outbuf));
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

#ifdef INCLUDE_CDC_INTERFACE
static const struct usb_endpoint_descriptor cdc_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor cdc_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 2
    #ifdef INCLUDE_DFU_INTERFACE
      +1
    #endif
    ,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 1
    #ifdef INCLUDE_DFU_INTERFACE
      +1
    #endif
    ,
		.bSubordinateInterface0 = 2
    #ifdef INCLUDE_DFU_INTERFACE
      +1
    #endif
    ,
	 }
};

static const struct usb_interface_descriptor cdc_comm_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1
  #ifdef INCLUDE_DFU_INTERFACE
    +1
  #endif
  ,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = cdc_comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors)
};

static const struct usb_interface_descriptor cdc_data_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 2
  #ifdef INCLUDE_DFU_INTERFACE
    +1
  #endif
  ,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = cdc_data_endp,
};
#endif

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
#ifdef INCLUDE_CDC_INTERFACE
}, {
  .num_altsetting = 1,
    .altsetting = &cdc_comm_iface,
}, {
  .num_altsetting = 1,
    .altsetting = &cdc_data_iface,
#endif
}};

const struct usb_config_descriptor config = {
  .bLength = USB_DT_CONFIGURATION_SIZE,
  .bDescriptorType = USB_DT_CONFIGURATION,
  .wTotalLength = 0,
  .bNumInterfaces = 1
  #ifdef INCLUDE_DFU_INTERFACE
    +1
  #endif
  #ifdef INCLUDE_CDC_INTERFACE
    +2
  #endif
  ,
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

#ifdef INCLUDE_CDC_INTERFACE
static int cdcacm_control_request(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)dev;

	switch(req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xA1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;
		// usbd_ep_write_packet(0x83, buf, 10);
		return 1;
		}
	case USB_CDC_REQ_SET_LINE_CODING: 
		if(*len < sizeof(struct usb_cdc_line_coding))
			return 0;

		return 1;
	}
	return 0;
}

struct output_task {
	fibre_t fibre;
	uint8_t ch;
};
static int output_fibre(fibre_t *fibre)
{
	struct output_task *t = containerof(fibre, struct output_task, fibre);
	PT_BEGIN_FIBRE(fibre);

	/* Initial timeout to allow things to settle. This proved necessary
	 * to make coldboot connections work on STM32F4-Discovery (issue was
	 * not deeply investigated but likely to be due to trying to send
	 * serial output whilst the usb thread is going through the hotplug
	 * sequence).
	 */
	if (time64_now() < 2000000)
		PT_WAIT_UNTIL(fibre_timeout(2000000));

	while (true) {
		int ch = ringbuf_get(&outring);
		if (-1 == ch)
			PT_EXIT();

		t->ch = ch;

		while (!usbd_ep_write_packet(usbd_dev, 0x82, &t->ch, 1))
			PT_YIELD();

	}

	PT_END();
}
static struct output_task output_task = {
	.fibre = FIBRE_VAR_INIT(output_fibre)
};

static void cdcacm_data_rx_cb(usbd_device *dev, uint8_t ep)
{
	(void)ep;

	char buf[64];
	int len = usbd_ep_read_packet(dev, 0x01, buf, 64);

	for (int i = 0; i < len; i++) {
		(void)ringbuf_put(&outring, buf[i]);
		if (buf[i] == '\r') {
			(void)ringbuf_put(&outring, '\n');
			console_putchar(&console, '\n');
		} else {
			console_putchar(&console, buf[i]);
		}
	}
	fibre_run_atomic(&output_task.fibre);
}

int _write(int fd, char *ptr, int len)
{
	if (fd == 1 || fd == 2) {
		for (int i=0; i<len; i++) {
			if (ptr[i] == '\n')
				(void) ringbuf_put(&outring, '\r');
			(void) ringbuf_put(&outring, ptr[i]);
		}
		//fibre_run(&output_task.fibre);
		return 0;
	}

	errno = EIO;
	return -1;
}
#endif

static void hid_set_config(usbd_device *dev, uint16_t wValue)
{
  (void)wValue;
  (void)dev;

  usbd_ep_setup(dev, 0x81, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);

  #ifdef INCLUDE_CDC_INTERFACE
  usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
  usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
  usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);
  #endif

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
#ifdef INCLUDE_CDC_INTERFACE
  usbd_register_control_callback(
      dev,
      USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
      USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
      cdcacm_control_request);
#endif

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

  fibre_run(&usb_task);
}

void usb_write(uint8_t *bytes, uint32_t len) {
  usbd_ep_write_packet(usbd_dev, 0x81, bytes, len);
}
