//
// Created by jakechen on 2017/7/7.
//

#include <Log.h>
#include "SoundTouchJni.h"


const int MAX_TRACKS = 16;
vector<SoundTouchStream> stStreams(MAX_TRACKS);

static void *getConvBuffer(int);

static int write(const float *, queue<jbyte> *, int, int);

static void setup(SoundTouchStream &, int, int, int, float, float);

static void convertInput(jbyte *, float *, int, int);

static inline int saturate(float, float, float);

static void *getConvBuffer(int);

static int process(SoundTouchStream &, SAMPLETYPE *, queue<jbyte> *, int, bool);

static void setPitchSemi(SoundTouchStream &, float);

static void setTempo(SoundTouchStream &, float);

static void setRate(SoundTouchStream &, float);

static void setTempoChange(SoundTouchStream &, float);

static void setRateChange(SoundTouchStream &, float);

static int copyBytes(jbyte *, queue<jbyte> *, int);

static int copyBytes(jbyte *arrayOut, queue<jbyte> *byteBufferOut, int toGet) {
    int bytesWritten = 0;

    for (int i = 0; i < toGet; i++) {
        if (byteBufferOut->size() > 0) {
            arrayOut[i] = byteBufferOut->front();
            byteBufferOut->pop();
            ++bytesWritten;
        } else {
            break;
        }
    }

    return bytesWritten;
}

static int process(SoundTouchStream &soundTouch, SAMPLETYPE *fBufferIn,
                   queue<jbyte> *byteBufferOut, const int BUFF_SIZE, bool finishing) {
    const int channels = soundTouch.getChannels();
    const int buffSizeSamples = BUFF_SIZE / channels;
    const int bytesPerSample = soundTouch.getBytesPerSample();
    int nSamples = BUFF_SIZE / channels;

    int processed = 0;
    if (finishing) {
        soundTouch.flush();
    } else {
        soundTouch.putSamples(fBufferIn, nSamples);
    }
    do {
        nSamples = soundTouch.receiveSamples(fBufferIn, buffSizeSamples);
        processed += write(fBufferIn, byteBufferOut, nSamples * channels,
                           bytesPerSample);
    } while (nSamples != 0);
    return processed;
}

static void *getConvBuffer(int sizeBytes) {
    int convBuffSize = (sizeBytes + 15) & -8;
    // round up to following 8-byte bounday
    char *convBuff = new char[convBuffSize];
    return convBuff;
}

static int write(const float *bufferIn, queue<jbyte> *bufferOut, int numElems,
                 int bytesPerSample) {
    int numBytes;

    int oldSize = bufferOut->size();

    if (numElems == 0)
        return 0;

    numBytes = numElems * bytesPerSample;
    short *temp = (short *) getConvBuffer(numBytes);

    switch (bytesPerSample) {
        case 1: {
            unsigned char *temp2 = (unsigned char *) temp;
            for (int i = 0; i < numElems; i++) {
                temp2[i] = (unsigned char) saturate(bufferIn[i] * 128.0f + 128.0f,
                                                    0.0f, 255.0f);
            }
            break;
        }

        case 2: {
            short *temp2 = (short *) temp;
            for (int i = 0; i < numElems; i++) {
                short value = (short) saturate(bufferIn[i] * 32768.0f, -32768.0f,
                                               32767.0f);
                temp2[i] = value;
            }
            break;
        }

        case 3: {
            char *temp2 = (char *) temp;
            for (int i = 0; i < numElems; i++) {
                int value = saturate(bufferIn[i] * 8388608.0f, -8388608.0f,
                                     8388607.0f);
                *((int *) temp2) = value;
                temp2 += 3;
            }
            break;
        }

        case 4: {
            int *temp2 = (int *) temp;
            for (int i = 0; i < numElems; i++) {
                int value = saturate(bufferIn[i] * 2147483648.0f, -2147483648.0f,
                                     2147483647.0f);
                temp2[i] = value;
            }
            break;
        }
        default:
            //should throw
            break;
    }
    for (int i = 0; i < numBytes / 2; ++i) {
        bufferOut->push(temp[i] & 0xff);
        bufferOut->push((temp[i] >> 8) & 0xff);
    }
    delete[] temp;
    temp = NULL;
    return bufferOut->size() - oldSize;
}

static void setPitchSemi(SoundTouchStream &soundTouch, float pitchSemi) {
    soundTouch.setPitchSemiTones(pitchSemi);
}

static void setTempo(SoundTouchStream &soundTouch, float tempo) {
    soundTouch.setTempo(tempo);
}

static void setRate(SoundTouchStream &soundTouch, float rate) {
    soundTouch.setRate(rate);
}

static void setTempoChange(SoundTouchStream &soundTouch, float tempoChange) {
    soundTouch.setTempoChange(tempoChange);
}

static void setRateChange(SoundTouchStream &soundTouch, float rateChange) {
    soundTouch.setRateChange(rateChange);
}

static void setup(SoundTouchStream &soundTouch, int channels, int sampleRate,
                  int bytesPerSample, float tempoChange, float pitchSemi) {
    soundTouch.setBytesPerSample(bytesPerSample);
    soundTouch.setSampleRate(sampleRate);
    soundTouch.setChannels(channels);
    soundTouch.setTempo(tempoChange);
    soundTouch.setPitchSemiTones(pitchSemi);
    soundTouch.setRateChange(0);
    soundTouch.setSetting(SETTING_USE_QUICKSEEK, false);
    soundTouch.setSetting(SETTING_USE_AA_FILTER, true);
}

static void convertInput(jbyte *input, float *output, const int BUFF_SIZE,
                         int bytesPerSample) {
    switch (bytesPerSample) {
        case 1: {
            unsigned char *temp2 = (unsigned char *) input;
            double conv = 1.0 / 128.0;
            for (int i = 0; i < BUFF_SIZE; i++) {
                output[i] = (float) (temp2[i] * conv - 1.0);
            }
            break;
        }
        case 2: {
            short *temp2 = (short *) input;
            double conv = 1.0 / 32768.0;
            for (int i = 0; i < BUFF_SIZE; i++) {
                short value = temp2[i];
                output[i] = (float) (value * conv);
            }
            break;
        }
        case 3: {
            char *temp2 = (char *) input;
            double conv = 1.0 / 8388608.0;
            for (int i = 0; i < BUFF_SIZE; i++) {
                int value = *((int *) temp2);
                value = value & 0x00ffffff;             // take 24 bits
                value |= (value & 0x00800000) ? 0xff000000 : 0; // extend minus sign bits
                output[i] = (float) (value * conv);
                temp2 += 3;
            }
            break;
        }
        case 4: {
            int *temp2 = (int *) input;
            double conv = 1.0 / 2147483648.0;
            assert(sizeof(int) == 4);
            for (int i = 0; i < BUFF_SIZE; i++) {
                int value = temp2[i];
                output[i] = (float) (value * conv);
            }
            break;
        }
    }
}

static inline int saturate(float fvalue, float minval, float maxval) {
    if (fvalue > maxval) {
        fvalue = maxval;
    } else if (fvalue < minval) {
        fvalue = minval;
    }
    return (int) fvalue;
}

JNI(void, clearBytes)(JNIEnv *env, jclass type, jint track) {
    SoundTouchStream &soundTouch = stStreams.at(track);

    const int BUFF_SIZE = 8192;

    queue<jbyte> *byteBufferOut = soundTouch.getStream();

    SAMPLETYPE *fBufferIn = new SAMPLETYPE[BUFF_SIZE];
    soundTouch.clear();

    delete[] fBufferIn;
    fBufferIn = NULL;

    while (!byteBufferOut->empty()) {
        byteBufferOut->pop();
    }
}

JNI(void, setup)(JNIEnv *env, jobject thiz, jint track, jint channels, jint samplingRate,
                 jint bytesPerSample, jfloat tempo, jfloat pitchSemi) {
    SoundTouchStream &stream = stStreams.at(track);;
    setup(stream, channels, samplingRate, bytesPerSample, tempo, pitchSemi);
}

JNI(void, finish)(JNIEnv *env, jobject thiz, jint track, int length) {
    SoundTouchStream &soundTouch = stStreams.at(track);

    const int bytesPerSample = soundTouch.getBytesPerSample();
    const int BUFF_SIZE = length / bytesPerSample;

    queue<jbyte> *byteBufferOut = soundTouch.getStream();

    SAMPLETYPE *fBufferIn = new SAMPLETYPE[BUFF_SIZE];
    process(soundTouch, fBufferIn, byteBufferOut, BUFF_SIZE, true); //audio is finishing

    delete[] fBufferIn;
    fBufferIn = NULL;
}

JNI(void, putBytes)(JNIEnv *env, jobject thiz, jint track, jbyteArray input, jint length) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    const int bytesPerSample = soundTouch.getBytesPerSample();
    const int BUFF_SIZE = length / bytesPerSample;
    queue<jbyte> *byteBufferOut = soundTouch.getStream();
    jboolean isCopy;
    jbyte *ar = env->GetByteArrayElements(input, &isCopy);
    SAMPLETYPE *fBufferIn = new SAMPLETYPE[BUFF_SIZE];
    convertInput(ar, fBufferIn, BUFF_SIZE, bytesPerSample);
    process(soundTouch, fBufferIn, byteBufferOut, BUFF_SIZE, false); //audio is ongoing.
    env->ReleaseByteArrayElements(input, ar, JNI_ABORT);
    delete[] fBufferIn;
    fBufferIn = NULL;
}

JNI(jint, getBytes)(JNIEnv *env, jobject thiz, jint track, jbyteArray get, jint toGet) {
    queue<jbyte> *byteBufferOut = stStreams.at(track).getStream();

    jbyte *res = new jbyte[toGet];

    jint bytesWritten;

    jboolean isCopy;
    jbyte *ar = (jbyte *) env->GetPrimitiveArrayCritical(get, &isCopy);

    bytesWritten = copyBytes(ar, byteBufferOut, toGet);

    env->ReleasePrimitiveArrayCritical(get, ar, JNI_ABORT);

    delete[] res;
    res = NULL;

    return bytesWritten;
}

JNI(void, setPitchSemi)(JNIEnv *env, jobject thiz, jint track, jfloat pitchSemi) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    setPitchSemi(soundTouch, pitchSemi);
}

JNI(void, setTempo)(JNIEnv *env, jobject thiz, jint track, jfloat tempo) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    setTempo(soundTouch, tempo);
}

JNI(void, setRate)(JNIEnv *env, jobject thiz, jint track, jfloat rate) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    setRate(soundTouch, rate);
}

JNI(void, setRateChange)(JNIEnv *env, jobject thiz, jint track, jfloat rate) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    setRateChange(soundTouch, rate);
}

JNI(jlong, getOutputBufferSize)(JNIEnv *env, jobject thiz, jint track) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    queue<jbyte> *byteBufferOut = soundTouch.getStream();
    return byteBufferOut->size();
}

JNI(void, setTempoChange)(JNIEnv *env, jobject thiz, jint track, jfloat tempoChange) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    setTempoChange(soundTouch, tempoChange);
}

JNI(void, setSpeech)(JNIEnv *env, jobject thiz, jint track, jboolean speech) {
    SoundTouchStream &soundTouch = stStreams.at(track);
    if (speech) {
        // use settings for speech processing
        soundTouch.setSetting(SETTING_SEQUENCE_MS, 40);
        soundTouch.setSetting(SETTING_SEEKWINDOW_MS, 15);
        soundTouch.setSetting(SETTING_OVERLAP_MS, 8);
        //fprintf(stderr, "Tune processing parameters for speech processing.\n");
    } else {
        soundTouch.setSetting(SETTING_SEQUENCE_MS, 0);
        soundTouch.setSetting(SETTING_SEEKWINDOW_MS, 0);
        soundTouch.setSetting(SETTING_OVERLAP_MS, 8);
    }
}