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

In theory you should be able to connect the 8Bitdo Retro Receiver to this setup,
making PS3/PS4/Wii Mote and Wii U Pro controllers compatible with the Switch (untested).

### NES/SNES Controller

The NES/SNES controller should be connected to GPIO A0/DATA, A1/LATCH and A2/CLOCK. Power should just be connected between 3.3V and GND, don't use the 5V output!

I actually put a board into one of my SNES controllers ;)

