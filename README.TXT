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

