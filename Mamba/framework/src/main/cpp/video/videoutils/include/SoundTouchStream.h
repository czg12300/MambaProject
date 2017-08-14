//
// Created by jakechen on 2017/8/10.
//

#ifndef MAMBA_SOUNDTOUCHSTREAM_H
#define MAMBA_SOUNDTOUCHSTREAM_H

#include "libsoundtouch/SoundTouch.h"

#include <queue>
#include <jni.h>
#include <Log.h>

using namespace std;
using namespace soundtouch;
namespace video {

    class SoundTouchStream {

    private:
        queue<jbyte> *byteBufferOut;
        uint sampleRate;
        int bytesPerSample;
        SoundTouch *soundTouch;
        int channels;
    public:

        queue<jbyte> *getStream() {
            return byteBufferOut;
        }

        int getSampleRate() {
            return sampleRate;
        }

        int getBytesPerSample() {
            return bytesPerSample;
        }

        void setBytesPerSample(int bytesPerSample) {
            this->bytesPerSample = bytesPerSample;
        }

        int getChannels() {
            return channels;
        }

        SoundTouchStream() {
            byteBufferOut = new queue<jbyte>();
            bytesPerSample = 0;
            sampleRate = 0;
            soundTouch = new SoundTouch();
        }

        SoundTouchStream(const SoundTouchStream &other) {
            byteBufferOut = new queue<jbyte>();
            bytesPerSample = 0;
            sampleRate = 0;
            soundTouch = new SoundTouch();
        }

        ~SoundTouchStream() {
            delete (soundTouch);
            delete (byteBufferOut);
        }

        void setRate(double newRate) {
            soundTouch->setRate(newRate);
        }

        void setTempo(double newTempo) {
            soundTouch->setTempo(newTempo);
        }

        void setRateChange(double newRate) {
            soundTouch->setRateChange(newRate);
        }

        void setTempoChange(double newTempo) {
            soundTouch->setTempoChange(newTempo);
        }

        void setPitch(double newPitch) {
            soundTouch->setPitch(newPitch);
        }

        void setPitchOctaves(double newPitch) {
            soundTouch->setPitchOctaves(newPitch);
        }


        void setPitchSemiTones(int newPitch) {
            soundTouch->setPitchSemiTones(newPitch);
        }

        void setPitchSemiTones(double newPitch) {
            soundTouch->setPitchSemiTones(newPitch);
        }

        void setChannels(uint numChannels) {
            soundTouch->setChannels(numChannels);
            channels = numChannels;
            LOGD("setChannels  numChannels %d  channels   %d", numChannels, channels);
        }

        void setSampleRate(uint srate) {
            soundTouch->setSampleRate(srate);
            this->sampleRate = srate;
        }

        void flush() {
            soundTouch->flush();
        }

        void putSamples(const SAMPLETYPE *samples, uint numSamples) {
            soundTouch->putSamples(samples, numSamples);
        }

        uint receiveSamples(SAMPLETYPE *output, uint maxSamples) {
            return soundTouch->receiveSamples(output, maxSamples);
        }

        uint receiveSamples(uint maxSamples) {
            return soundTouch->receiveSamples(maxSamples);
        }

        void clear() {
            soundTouch->clear();
        }

        bool setSetting(int settingId, int value) {
            return soundTouch->setSetting(settingId, value);
        }

        int getSetting(int settingId) const {
            return soundTouch->getSetting(settingId);
        }

        uint numUnprocessedSamples() const {
            return soundTouch->numUnprocessedSamples();
        }

    };
}
#endif //MAMBA_SOUNDTOUCHSTREAM_H
