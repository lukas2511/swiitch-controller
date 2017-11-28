# sWiitch controller

Code to use some classic (or generally alternative) controllers on the Nintendo Switch.

Build using libopencm3 for the stm32f103 series microprocessor.

## Hardware

I'm using a stm32f108c8t6 on a so called "blue pill" board, which basically is just the bare minimum
to get the chip running.
I had to solder a 1.8kOhm resistor on the board between A12 and 3.3V to get USB working (see [stm32duino wiki](http://wiki.stm32duino.com/index.php?title=Blue_Pill#Hardware_installation)).

### Wii Classic Controller

The Wii Classic Controller should be connected to I2C1 (GPIO B6/SCL and B7/SDA, with around 4.7kOhm pull-up resistors).

You can use an extension cord to get a nice socket, or you can get one of the many cheap breakout boards, or even just splice into the wire of your controller,
or go haywire and build the stm32 board directly into the controller, the possibilities are endless!

Mine has the connector from a broken Wiimote :)

### NES/SNES Controller

The NES/SNES controller should be connected to GPIO A0/DATA, A1/LATCH and A2/CLOCK. Power should just be connected between 3.3V and GND, don't use the 5V output!

I actually put a board into one of my SNES controllers ;)

## Bootloader

I'm using Daniel Thompson's modified version of the example libopencm3 bootloader, which can be found in the [i2c-star repository](https://github.com/daniel-thompson/i2c-star/tree/master/src/bootloader).

I changed VID/PID to match the controller so dfu-util can automatically reprogram the device without a manual reset:

```diff
diff --git a/src/bootloader/usbdfu.c b/src/bootloader/usbdfu.c
index ba6ce07..97be28a 100644
--- a/src/bootloader/usbdfu.c
+++ b/src/bootloader/usbdfu.c
@@ -54,8 +54,8 @@ const struct usb_device_descriptor dev = {
        .bDeviceSubClass = 0,
        .bDeviceProtocol = 0,
        .bMaxPacketSize0 = 64,
-       .idVendor = 0x0483,
-       .idProduct = 0xDF11,
+       .idVendor = 0x0f0d,
+       .idProduct = 0x00c1,
        .bcdDevice = 0x0200,
        .iManufacturer = 1,
        .iProduct = 2,
```
