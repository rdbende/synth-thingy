synth: synth.cpp
	g++ synth.cpp RtAudio.cpp RtMidi.cpp -o synth -Wall -D__LINUX_ALSA__ -D__LITTLE_ENDIAN__ -lpthread -lasound

install:
	systemctl stop synth
	cp synth /usr/local/bin/synth
	systemctl start synth
