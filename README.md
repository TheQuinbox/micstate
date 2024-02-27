# Micstate

Simple Win32 program to speak the muted state of your default input device using a global hotkey.

## Why?

The current way of determining if an input device is muted or not is incredibly annoying:

1. Open mmsys.cpl.
2. Find the recording tab.
3. Find your device.
4. Press Alt+P.
5. Control tab until you find the listen tab.
6. Check listen to this device.
7. Press Alt+A.
8. Listen, and determine if it is or not, finally.
9. Uncheck the box and close all the dialogs you just opened.

I have a laptop with a deticated mute key that's super easy to hit, and I've hit it by accident many times, initially leading to my great confusion when no one could hear me, and later my paranoia that I hit it every time I launched a program that used the microphone. Hence, this program was born. Once you run it, it will passively sit in the background, waiting for you to press Control+Alt+Shift+M. When you do, the current state of your default input device will be spoken.

## Limitations

This program is sometimes currently unable to tell if a microphone has been muted at the hardware level. For example, if I mute my studio microphone with the mute button, it will report that, but if I mute my headset mic by pressing the physical mute button instead of doing it through Windows, this program will still report that it's unmuted. This is a limitation of the CoreAudio API and microphones not sending proper messages to the operating system, and not something I can work around as far as I'm aware. Suggestions appreciated!

## Building

I'm still very new to CMake, and even more so to needing to make my builds easily reproducible. As such, the current CMakeLists in this repository would probably give any CMake maintainer a heart attack. The only external dependency you should need is [Tolk](https://github.com/dkager/Tolk).

The only lines you should need to change in the CMakeLists file are the `target_include_directories` and `target_link_directories` calls to point to wherever you have Tolk.h and Tolk.lib, respectively. I keep mine in C:\dev\include and C:\dev\lib, but it's completely up to you where you put them, as long as you remember to update the paths. I appologise for this incredibly kludgy setup, and hope to improve it at some point in the future. This is yet another area where I'm very actively open to suggestions.
