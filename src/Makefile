BINARY = swiitch-controller

CONTROLLER ?= wiiclassic

OBJS += myconsole.o i2c.o
OBJS += $(CONTROLLER).o
OBJS += usb.o usb_cdc.o usb_dfu.o usb_hid.o
CPPFLAGS += -I include -I usb/include

vpath %.c usb
vpath %.c controllers

include ../lib/libopencm3.stm32f1xx.mk
