# XPThrottleFPS
X-Plane plugin that throttles X-Plane's CPU/GPU usage by suspending it for some microseconds each frame.

It might not be the proper way to do it, but it works for me and I am finally able to run X-Plane on my MacBookPro 16' i9 without the fans running at fullspeed. By targetting 25 FPS (default) the sim still runs smoothely wihtout any noticable problems as far as I can tell. Note that I'm using the POSIX usleep(microsecs) call which apparently only exists on linux and macOS, so it probably won't even compile on Windows.



Note that I learned that there exists a command-line argument `X-Plane.exe --lock_fr=030` which appeared to be just what I needed, but at least on macOS the CPU/GPU usage is not being reduced although the FPS indeed are locked to 30 in that example.


## Installation
You need cmake and then you can build it like so:

	mkdir build
	cd build
	cmake ..
	make
	
Then copy the file XPThrottleFPS.xpl to XPLANE/Resources/plugins/

You should then configure a keyboard command to enable the plugin and to set the target FPS. The following commands are available (with the keyboard commands I personally use):


![](screenshot.png)


