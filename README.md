# sWiitch controller

Code to use a Wii classic controller on the Nintendo Switch

Build using libopencm3 for the stm32f103 series.

## Hardware

I'm using a stm32f108c8t6 on a so called "blue pill" board, which basically is just the bare minimum
to get the chip running.
I had to solder a resistor on the board to get USB working (see [stm32duino wiki](http://wiki.stm32duino.com/index.php?title=Blue_Pill#Hardware_installation)).

Other than that a Wii extension adapter is connected to I2C1 (GPIO B6/B7 with 4.7kOhm pull-up resistors).

![hardware](https://github.com/lukas2511/swiitch-controller/raw/master/pics/17-11-23%2017-50-54%201132.jpg)

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
