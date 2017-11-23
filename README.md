# sWiitch controller

Code to use a Wii classic controller on the Nintendo Switch

Build using libopencm3 for the stm32f103 series.

## Hardware

I'm using a stm32f108c8t6 on a so called "blue pill" board, which basically is just the bare minimum
to get the chip running.
I had to solder a resistor on the board to get USB working (see [stm32duino wiki](http://wiki.stm32duino.com/index.php?title=Blue_Pill#Hardware_installation)).

Other than that a Wii extension adapter is connected to I2C1 (GPIO B6/B7 with 4.7kOhm pull-up resistors).

![hardware](https://github.com/lukas2511/swiitch-controller/raw/master/pics/17-11-23%2017-50-54%201132.jpg)
