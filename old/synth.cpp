#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <unistd.h>

#include "RtAudio.h"
#include "RtMidi.h"

using namespace std;

RtAudio *audioOut;
RtMidiIn *midiIn;

std::vector<unsigned char> message;
int nBytes, i;
double stamp;
double freq;
int timec = 0;
double pan = 0.5;

#define MIDI_DEVICE "Arturia MiniLab"
#define NOTE_ON 0x90
#define NOTE_OFF 0x80

double attack_time = 0.01;
double decay_time = 0.7;
double sustain_amp = 0.0;
double release_time = 0.4;
double start_amp = 1.0;


int findMidiPort() {
    unsigned int nPorts = midiIn->getPortCount();
    for (unsigned int i = 0; i < nPorts; i++) {
        if (midiIn->getPortName(i).find(MIDI_DEVICE) != string::npos) return i;
    }

    cout << "MIDI device not found. Available ports:\n\n";
    for (unsigned int i = 0; i < nPorts; i++) {
        cout << "  " << midiIn->getPortName(i) << endl;
    }

    return 0;
}

void initDevices() {
    audioOut = new RtAudio();

    if (audioOut->getDeviceCount() < 1) {
        cout << "No available audio device!" << endl;
        delete audioOut;
        exit(0);
    }

    midiIn = new RtMidiIn();
    midiIn->ignoreTypes(true, true, true);
    midiIn->openPort(findMidiPort());

    if (!midiIn->isPortOpen()) {
        delete audioOut;
        delete midiIn;
        exit(0);
    }
}

double releaseofftime = 0.0;

int tick(
    void *outBuffer,
    void *inBuffer,
    unsigned int nFrames,
    double streamTime,
    RtAudioStreamStatus status,
    void *userData
) {
    double *buffer = (double *) outBuffer;

    float s, v;
    float amplitude;
    float lifetime;

    for (unsigned int i = 0; i < nFrames; i++) {
        midiIn->getMessage(&message);
        nBytes = message.size();

        if (nBytes > 0) {
            // for (i=0; i < nBytes; i++)
            //   std::cout << (int)message[i] << " ";
            // std::cout << std::endl;

            if ((int)message[0] == NOTE_ON) {
                timec = 0;
                freq = 440.0 * powf(2.0, ((int)message[1] - 69) / 12.0);
            } else if ((int)message[0] == NOTE_OFF) {
    //            double step = buffer[nFrames - 1] / (double)nFrames;
    //            double v = buffer[nFrames - 1];
    //
    //            for (unsigned int i = 0; i < nFrames; i++) {
    //                v = v - step;
    //                buffer[i*2] = v;
    //                if (v == 0) break;
    //            }
                releaseofftime = timec;
            } else if ((int)message[1] == 110) {
                int value = (int)message[2];
                if (value < 16) {
                    pan = fmax(0, pan - 0.01);
                } else if (value > 16) {
                    pan = fmin(1, pan + 0.01);
                }
            } else if ((int)message[1] == 86) {
                pan = 0.5;
            }
        }
        {
            lifetime = timec / 44100.0;
            if (lifetime <= attack_time) {
                amplitude = (lifetime / attack_time);
            } else if (lifetime <= (attack_time + decay_time)) {
                amplitude = ((lifetime - attack_time) / decay_time) * (sustain_amp - start_amp) + 1;
            } else if (lifetime > (attack_time + decay_time)) {
                amplitude = sustain_amp;
            }
            s = sin(2 * M_PI * freq * lifetime);
            v = s * amplitude;
            buffer[i*2] = v * (1 - pan);
            buffer[i*2+1] = v * pan;
            timec++;
        }
    }

    return 0;
}


int main() {
    initDevices();

    RtAudio::StreamParameters *parameters = new RtAudio::StreamParameters();
    parameters->deviceId = audioOut->getDefaultOutputDevice();
    parameters->nChannels = 2;
    parameters->firstChannel = 0;

    unsigned int nFrames = 512;

    try {
        audioOut->openStream(parameters, NULL, RTAUDIO_FLOAT64, 44100, &nFrames, &tick, NULL);
        audioOut->startStream();
    } catch (RtAudioErrorType) {
        exit(0);
    }
    cout << ::getpid() << endl;  
    while (1) {
        ;
    }
    char input;
    cout << "Press enter to exit" << endl;
    cin.get(input);

    try {
        audioOut->stopStream();
    } catch (RtAudioErrorType) {
        ;
    }

    if (audioOut->isStreamOpen()) audioOut->closeStream();

    return 0;
}
