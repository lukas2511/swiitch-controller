BINARY = swiitch-controller

OBJS += wiiclassic.o myconsole.o i2c.o
OBJS += usb.o usb_cdc.o usb_dfu.o usb_hid.o
CPPFLAGS += -I include -I usb/include

vpath %.c usb

include ../lib/libopencm3.stm32f1xx.mk
