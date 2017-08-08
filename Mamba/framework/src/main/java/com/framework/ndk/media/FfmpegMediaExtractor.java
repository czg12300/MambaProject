package com.framework.ndk.media;

import android.media.MediaDataSource;
import android.media.MediaExtractor;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

/**
 * ffmpeg实现的media extractor
 *
 * @author jake
 * @since 2017/7/20 上午10:06
 */

public class FfmpegMediaExtractor {


    public FfmpegMediaExtractor() {
        nativeSetup();
//        String
//        MediaExtractor
    }

    private final native void nativeSetup();

    public native void setDataSource(String file) throws IOException;

    public native void seekTo(int timeUs, int mode);

    public native int getTrackCount();

    public FfmpegMediaFormat getTrackFormat(int index) {
        return nativeGetTrackFormat(index);
    }

    private native FfmpegMediaFormat nativeGetTrackFormat(int index);

    public native int getTimestamp();

    public native byte[] readSampleData();


    public native int readSampleData1(byte[] data);

    public native int getSampleDataSize();

    public native void selectTrack(int index);

    public native int getTrackIndex();

    public native int getDuration();

    public native void release();
}
