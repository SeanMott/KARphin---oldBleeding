# KARphin - A ~~GameCube and Wii~~ Kirby Air Ride Emulator

[Netplay Website](https://www.kirbyairrideonline.com/) | [Netplay Discord]() | [Wiki](https://kar.miraheze.org/wiki/Main_Page) | [Netplay FAQ]() | [KAR Workshop]()

KARphin is an emulator for running Kirby Air Ride Netplay on Windows,
Linux, macOS, and Android. It's licensed under the terms
of the GNU General Public License, version 2 or later (GPLv2+).

We recommend using [KAR Workshop]() for downloading both KARphin and rest of the needed Netplay tools.

## Why can't I just use normal Dolphin?

KARphin offers a few updates and tweaks specific to this game. We used to use a old Slippie build, but it got so out of date and the Linux build stopped working. So now we got a fancy (as of 04-19-2024) modern Dolphin build with some bells and whistles.

## What if I haven't played KAR ever or not since I was a kid?

Don't you worry KAR grand champ to be, we got a few tutorials and links to how the game plays and some meta videos.

- [What Are the Patches?](https://www.youtube.com/watch?v=awdgofcJlbc)

- [Machines Explained](https://www.youtube.com/watch?v=E48DGPncK8g)

- [KAR Is A Fighting Game!](https://www.youtube.com/watch?v=TDafSNoOXT4)

- [Rules For Netplay](https://www.kirbyairrideonline.com/leaderboards)

And don't forget to ask in the Discord, the best way to learn is play with others. So see ya in the City, champ in the making!

## Video Backends

- Vulkan (runs on Mac)
- Direct 3D (D3D) and DirectX 12 (D312) only available on Windows.
- Software Rendering (Intended for Developer/Debug only)

## Audio Backends

- HLE (High Level Emulation) is faster but less accurate
- LLE (Low Level Emulation) is slower but close to perfect. Note that LLE has two submodes (Interpreter and Recompiler).

## System Requirements

### Desktop

* OS
    * Windows (10 or higher).
    * Linux.
    * macOS (10.15 Catalina or higher).
    * Unix-like systems other than Linux are not officially supported but might work.
* Processor
    * A CPU with SSE2 support.
    * A modern CPU (3 GHz and Dual Core, not older than 2008) is highly recommended.
* Graphics
    * A reasonably modern graphics card (Direct3D 11.1 / OpenGL 3.3).
    * A graphics card that supports Direct3D 11.1 / OpenGL 4.4 is recommended.

### Android

* OS
    * Android (5.0 Lollipop or higher).
* Processor
    * A processor with support for 64-bit applications (either ARMv8 or x86-64).
* Graphics
    * A graphics processor that supports OpenGL ES 3.0 or higher. Performance varies heavily with [driver quality](https://dolphin-emu.org/blog/2013/09/26/dolphin-emulator-and-opengl-drivers-hall-fameshame/).
    * A graphics processor that supports standard desktop OpenGL features is recommended for best performance.

### Web
 * Not happening, Web GL is too slow, maybe when [Web GPU](https://www.khronos.org/assets/uploads/developers/presentations/WebGL__WebGPU_Updates_2024-03.pdf) (yes, it's a real spec) is fully implemented.

### Switch
* For legal reasons, no I can not tell you how to get unlicensed Nintendo software running on the Switch on a public README.

<br>
<b>KARphin can only be installed on devices that satisfy the above requirements. Attempting to install on an unsupported device will fail and display an error message.</b>

## Why are there so many Branches on Git?

- Master || The bleeding edge of all our features and tweaks, the one all other branches get merged into once their purpose is finished.

- Release-xxxxx || The specific release for each of the versions ie 1.0.0, 2.543.2, 5.0, ect

- Jas Toys || A special branch for Jas's side experiments and things that might get put into the final Release or never see the light of day

- Bare Metal || A branch with the special Netplay code replacemnt stripped out and regular Dolphin with very few *aesthetic* and KAR tweaks.

## What if I wanted to compile it myself?

Well buddy, you and I got something in common. That's why the build instructions were moved to a dedicated folder, split out into their own file for ease of reading. Most of them are straight forward and what we use ourself. If you have a fix/improvement, we welcome issues and pull requests. If you make something neat, Jas likes cool software toys. He can be found through the KAR discord.

- [Windows Build](READMES_ASSETS\Building\Building_Windows.md)
- [Mac Build](READMES_ASSETS\Building\Building_Mac.md)
- [Linux Build](READMES_ASSETS\Building\Building_Linux.md)
- [Android Build](READMES_ASSETS\Building\Building_Android.md)
- [Switch Build](READMES_ASSETS\Building\Building_Switch.md)