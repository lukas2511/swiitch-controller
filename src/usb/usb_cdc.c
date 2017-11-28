#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "usb.h"
#include "usb_cdc.h"

#include <librfn/fibre.h>
#include <librfn/time.h>

#include <librfn/util.h>
#include <librfn/console.h>
#include <librfn/ringbuf.h>
#include <libopencm3/usb/cdc.h>
#include "myconsole.h"
int _write(int file, char *ptr, int len);

static uint8_t outbuf[1024];
static ringbuf_t outring = RINGBUF_VAR_INIT(outbuf, sizeof(outbuf));

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

static struct {
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
		.bDataInterface = 0,
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
		.bControlInterface = 0,
		.bSubordinateInterface0 = 0,
	 }
};

static struct usb_interface_descriptor cdc_comm_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
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

static struct usb_interface_descriptor cdc_data_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = cdc_data_endp,
};

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
		fibre_run(&output_task.fibre);
		return 0;
	}

	errno = EIO;
	return -1;
}

void init_usb_cdc(void) {
  cdcacm_functional_descriptors.cdc_union.bControlInterface = register_usb_interface_descriptor(&cdc_comm_iface);
  cdcacm_functional_descriptors.call_mgmt.bDataInterface = register_usb_interface_descriptor(&cdc_data_iface);
  cdcacm_functional_descriptors.cdc_union.bSubordinateInterface0 = cdcacm_functional_descriptors.call_mgmt.bDataInterface;

  register_usb_callback(USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE, USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, cdcacm_control_request);
  register_usb_endpoint(0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
  register_usb_endpoint(0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
  register_usb_endpoint(0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);
}
