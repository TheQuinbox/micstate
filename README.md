# Micstate

Simple C++ program to speak the muted state of your default input device using a global hotkey.

## Why?

The current way of determining if an input device is muted or not on Windows is incredibly annoying:

1. Open mmsys.cpl.
2. Find the recording tab.
3. Find your device.
4. Press Alt+P.
5. Control tab until you find the listen tab.
6. Check listen to this device.
7. Press Alt+A.
8. Listen, and determine if it is or not, finally.
9. Uncheck the box, hit apply, and close all the dialogs you just opened.

I have a laptop with a deticated mute key that's super easy to hit, and I've hit it by accident many times, initially leading to my great confusion when no one could hear me, and later my paranoia that I hit it every time I launched a program that used the microphone. Hence, this program was born. Once you run it, it will passively sit in the background, waiting for you to press Control+Alt+Shift+M. When you do, the current state of your default input device will be spoken.

## Configuration

The way this program works is by taking a tiny sample of audio (50 MS or so) from your default input device, getting the noise level (in DB), and comparing it to a configurable value. If the value is less than or equal to the configured value, the microphone is considered muted. By default, the value is -60 DB, but you can always change it. Open config.ini (generated when you run the program for the first time), and change the value of min_level in the DEFAULT section to your desired value. You'll have to relaunch the program for this change to take effect.

## Building from Source

I'm still very new to CMake, and even more so to needing to make my builds easily reproducible. As such, the current CMakeLists in this repository would probably give any CMake maintainer a heart attack. The external dependencies you'll need are:

1. [Tolk](https://github.com/dkager/Tolk)
2. [Bass](https://www.un4seen.com/bass.html)
3. [SimpleINI](https://github.com/brofield/simpleini)

The only lines you should need to change in the CMakeLists file are the `target_include_directories`, `target_link_directories`, and  `file` calls to point to wherever you have the required headers, lib files, and DLLs, respectively. I keep mine in C:\dev\include, C:\dev\lib, and C:\dev\bin, but it's completely up to you where you put them, as long as you remember to update the paths. I appologise for this incredibly kludgy setup, and hope to improve it at some point in the future. This is yet another area where I'm very actively open to suggestions.
