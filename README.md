# LaForge
Sensory expansion tools for seeing the invisible, hearing silence, and sensing the world beyond limited human sensors.

PDMmic -- Uses ultrasound capable digital MEMs microphone to capture ultrasound to wav files, and soon real time playback.
	  Captured ultrasound (and maybe infrasound) will be frequency shifted into the human audible range. I have made
	  a working prototype elsewhere which allows me to listen to 50KHz-ish bat calls shifted to 6KHz without time distortion.
	  The current code interfaces with a Knowles SPH0641LU4H through a Raspberry Pi SPI port. It doesn't work perfectly
	  yet. 
	  Status: Incomplete, a little buggy and very messy code.




# Extra Credits
These 3rd party source code files have been very helpful. Since I have modified at least some of them and forgot what
I did, and suspect the authors probably wouldn't be intested in having my hack jobs, there probably isn't much point
to using Git's submodules. Nevertheless, I want credit to go where credit is due since they saved me time.

CIC.cpp and CIC.h CIC Filter https://github.com/EsonJohn/CIC-filter
Wav.cpp and Wav.h Wav file writer http://blog.acipo.com/generating-wave-files-in-c/
