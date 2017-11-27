BINARY = swiitch-controller

OBJS += usb.o wiiclassic.o myconsole.o i2c.o

include lib/libopencm3.stm32f1xx.mk
