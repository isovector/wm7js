# wm7js

Back in 2010, after trying (and failing) to get [wm2js](https://trac.v2.nl/browser/andres/wm2js/) to work, I decided to whip up my own means of connecting and using wiimotes to a computer. In theory, it will connect up to 7 wiimotes. wm7js was designed to hook up to Project64, which might explain why the buttons map as oddly as they do.

I'm not proud of this code. It's nastily hacked together, but once upon a time it *did* work. It might still work!

## Getting it running

Build and install [wiiuse](https://github.com/rpavlik/wiiuse). `modprobe uinput`. Run `make`. Pray.

I don't entirely remember how it works, but I'm pretty sure you run `wm7js` and then start slapping the 1 and 2 buttons on the Wiimote.

## Configuring

Defining `DPAD_TO_ANALOG_STICK` in wm7js.c will map the d-pad to an analog joystick. By default, wm7js uses the left analogue stick.