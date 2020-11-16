# Doom 0.5heh

## What is it:
Quick and dirty reverse engineering project of the Doom Alpha v0.5.
Made for the sake of trying to document the nature of the Doom rendering code
before the introduction of BSP trees.

## Release notes:
This project has only been tested to compile with Visual Studio in the x86
configuration. I want to make it run on more platforms eventually. The project
requires SDL 2 (developed with 2.0.12) to build. Linux support will require
a little effort to smooth over the use of nonstandard fcntl.h and io.h 
functions.

The port is very barebones. There's no external configuration for the window
or controls at the moment. Resolution can be specified with the width and
height command line parameters. fullscreen will make it run fullscreen. You
can use novert to emulate the novert TSR for the mouse. Given the nature of the
port, this probably won't change unless someone sends in a patch.

The code is very raw at the moment. Cleanup might commence later to make it 
more readable, but it should at least work. There's plenty of bugs that remain
from the original alpha-quality code, but it doesn't seem to crash any more 
than the original alpha.

The IO code isn't particularly great. I incorporated a system to emulate chunky
VGA video memory writes, but in practice this doesn't do anything but slow 
things down. It could probably be removed, but I wanted to avoid altering the 
game code as much as possible.

I'd like to port this back to DOS at some point and fully RE the original IO 
code, but this will come later. 

Hires mode will invariably crash the game, but the high color mode is partially
emulated. There's no system in place to draw pics in high-color mode, so the UI
and menu will look very weird, but it'll at least function.

## License:
From similar RE projects, such as Chocolate Strife and Doom 64 EX, as well as
the fact that I referenced the released Doom and Heretic source on occasion,
I have chosen to place this under the GPL v2 for now. Details in copying.txt
