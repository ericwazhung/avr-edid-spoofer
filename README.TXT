Everything here, except where noted, is my own work. Feel free to use it as
you wish, but please give credit where it's due, and consider sending me a
beer! The only exception is if you intend to profit from it, or are part of
an organization, in which case you should contact me first.

No Spaces: Eric Waz Hung at gmail dot com
-----------

This project is designed to be used in leu of an EDID chip. 

Thus, you can create your own timings, or use a display that doesn't have
an EDID in a machine that needs one.

Specifically, it was designed for a dual-pixel Toshiba LTD121KM7K LCD
connected via single-pixel to dual-pixel converter, though it should be 
easy to modify for most any screen.

Note that the schematic was designed with the intention to connect to and
reinterpret an LCD's built-in EDID... this functionality was dropped, since
it turned out the display didn't have one.

The LED and pushbutton shown in the schematic are unnecessary. In fact, the
pushbutton is completely unimplemented.

Additionally, the avr-edid-spoofer has a bit of code for an RGB "LED-Fader"
which cycles between the three colors. The brightness of each LED can be 
set via the DDC channel, if desired, or it can be ignored completely.
(If you want to see how to control it, send me an email and I can pass on 
 the code)

Enjoy!

-------------------------------------------------

As-Of 11-27-15:
The official code-site has changed to github.
Below is the original description from google-code...
TODO: Merge this with the above!

This essentially turns an AVR ATtiny45 (or any AVR, with slight modification) into an EDID chip for displays which don't have them, or for creating your own timings, etc. It can also make use of the i2c DDC bus for other communication purposes.

Contains extensive notes re: EDID parameters, etc. Also, automatically generates EDID data-values based on timings as usually viewed/modified in a computer: H/V Front Porch, H/V Sync Width, H/V Back Porch (rather than "H/V Blank Period" as its stored in the data-array). The checksum is automatically generated, as well, upon boot.

This was developed for connection of a dual-pixel LCD to a laptop with a single-pixel LVDS transmitter via single-pixel to dual-pixel converter ( https://sites.google.com/site/geekattempts/home-1/lvds-single-to-dual-converter/lvds-single-to-dual-converter-1 ). So:

The original intent was to automatically read the EDID data from the dual-pixel display, and convert those values to their single-pixel equivalent to be transmitted to the computer. This was never implemented, as the display itself didn't have an EDID chip. And, further, timing-values had to be optimized for the data-rate limitations of the circuitry and single-pixel transmitter/cables (54MHz dual-pixel-clock -> 108MHz single-pixel-clock was too high for the system, and had to be dropped to 100MHz).

ALSO

Some values have been entered manually as hex-values (such as the values indicating "Digital Interface" and "Single-Pixel") and are only reconfigurable by reading the EDID specs. These values (should be) pretty-well-commented, so...

It should be a great starting-point regardless of your display's differing parameters.

It shouldn't be too difficult to modify this for any microcontroller which can be programmed in C. 
