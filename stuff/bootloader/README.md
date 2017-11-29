# Bootloader

Here you can find a binary release of the bootloader I'm using on my blue-pill.

## Code

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

## Other bootloaders / No bootloader

Basically any DFU bootloader should work, but you may need to add the special `remain-in-loader` flag
that Daniel Thompson's loader uses, or use a physical switch / button to enter the bootloader when you need it.

If you don't want to you don't even have to use a bootloader, just change the data address in the
linker script and you can use a blackmagic probe or stlink device to flash and debug your system,
or flash the stm32 through USART, I don't know, do whatever you like.
