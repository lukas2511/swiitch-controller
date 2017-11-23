/*
 * Part of librfn (a general utility library from redfelineninja.org.uk)
 *
 * This file was derived from libopencm3's cdcacm example.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2014 Daniel Thompson <daniel@redfelineninja.org.uk>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <librfn/console.h>
#include <librfn/fibre.h>
#include <librfn/ringbuf.h>
#include <librfn/time.h>
#include <librfn/util.h>

#ifndef CONFIG_USB_MAX_POWER
#define CONFIG_USB_MAX_POWER 100
#endif

static const struct usb_device_descriptor desc = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x6666,  /* Prototype product vendor ID */
	.idProduct = 0x9637, /* dd if=/dev/random bs=2 count=1 | hexdump */
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec it's
 * optional, but its absence causes a NULL pointer dereference in the
 * Linux cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
} };

static const struct usb_endpoint_descriptor data_endp[] = {{
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
} };

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
		.bDataInterface = 1,
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
		.bSubordinateInterface0 = 1,
	 }
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors)
} };

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
} };

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
} };

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = (CONFIG_USB_MAX_POWER+1) / 2,

	.interface = ifaces,
};

static char serial_no[25];

static const char * usb_strings[] = {
	"redfelineninja.org.uk",
	"CDC-ADM command console",
	serial_no
};

static console_t *console;
static uint8_t outbuf[1024];
static ringbuf_t outring = RINGBUF_VAR_INIT(outbuf, sizeof(outbuf));
static usbd_device *usbd_dev;

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

/* prototype functions not found in the headers (for -Wmissing-prototypes) */
int _write(int fd, char *ptr, int len);
static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue);

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

#if defined(STM32F1)
static void usb_hwinit(void)
{
	uint32_t t;

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);

	/* lower hotplug and leave enough time for the host to notice */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
		      GPIO11 | GPIO12);
	gpio_clear(GPIOA, GPIO11 | GPIO12);
	t = time_now() + 10000;
	while (cyclecmp32(time_now(), t) < 0)
		;

	/* use the device signature as the serial number */
	desig_get_unique_id_as_string(serial_no, sizeof(serial_no));

	/* hotplug will automatically be unasserted as the usb cell takes
	 * command of the pins.
	 */
	usbd_dev =
	    usbd_init(&st_usbfs_v1_usb_driver, &desc, &config, usb_strings, 3,
		      usbd_control_buffer, sizeof(usbd_control_buffer));
}
#elif defined(STM32F4)
static void usb_hwinit(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_OTGFS);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO9 | GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12);

	usbd_dev = usbd_init(&otgfs_usb_driver, &desc, &config,
			usb_strings, 2,
			usbd_control_buffer, sizeof(usbd_control_buffer));
}
#else
#error Unsupported part
#endif

static int usb_fibre(fibre_t *fibre)
{
	PT_BEGIN_FIBRE(fibre);

	usb_hwinit();

	usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

	while (true) {
		usbd_poll(usbd_dev);
		PT_YIELD();
	}

	PT_END();
}
static fibre_t usb_task = FIBRE_VAR_INIT(usb_fibre);

static int cdcacm_control_request(usbd_device *dev,
	struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
	void (**complete)(usbd_device *dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/* The Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		return 1;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding)) {
			return 0;
		}

		return 1;
	}
	return 0;
}

static void cdcacm_data_rx_cb(usbd_device *dev, uint8_t ep)
{
	(void)ep;

	char buf[64];
	int len = usbd_ep_read_packet(dev, 0x01, buf, 64);

	for (int i = 0; i < len; i++) {
		(void)ringbuf_put(&outring, buf[i]);
		if (buf[i] == '\r') {
			(void)ringbuf_put(&outring, '\n');
			console_putchar(console, '\n');
		} else {
			console_putchar(console, buf[i]);
		}
	}
	fibre_run_atomic(&output_task.fibre);
}

static void cdcacm_set_config(usbd_device *dev, uint16_t wValue)
{
	(void)wValue;

	usbd_ep_setup(dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64,
			cdcacm_data_rx_cb);
	usbd_ep_setup(dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
	usbd_ep_setup(dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(
	    dev, USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
	    USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT, cdcacm_control_request);
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

void console_hwinit(console_t *c)
{
	console = c;
	fibre_run(&usb_task);

	/* Going silent, which in this case means not speaking until spoken to,
	 * ensures the console doesn't send output until USB is fully settled
	 * (as proved because the host can send us data)
	 */
	console_silent(c);
}
