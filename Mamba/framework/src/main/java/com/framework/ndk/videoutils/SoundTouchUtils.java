package com.framework.ndk.videoutils;

/**
 * 使用sound touch来做声音效果
 *
 * @author jake
 * @since 2017/7/7 上午11:58
 */

public class SoundTouchUtils {
    public SoundTouchUtils() {
    }

    public native void setupAudioParameters(int sampleRate, int channels);

    public native int getSampleByBytes(byte[] samples, int size);

    public native void putSampleByBytes(byte[] samples, int size);

    public native void setTempo(double newTempo);

    public native void setTempoChange(double newTempo);

    public native void setPitch(double newPitch);

    public native void setPitchOctaves(double newPitch);

    public native void setPitchSemiTones(int newPitch);

    public native void setRateChange(double newRate);

    public native void setRate(double newRate);

    public native void init();

    public native void release();
}
