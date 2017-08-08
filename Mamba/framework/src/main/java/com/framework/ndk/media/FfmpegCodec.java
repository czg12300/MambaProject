package com.framework.ndk.media;

import java.nio.ByteBuffer;

/**
 * 基于ffmpeg的编解码器实现
 *
 * @author jake
 * @since 2017/7/24 上午10:51
 */

public class FfmpegCodec {
    public FfmpegCodec() {
        nativeSetup();
    }

    private native void nativeSetup();

    public void configure(FfmpegMediaFormat ffmpegMediaFormat) {
        if (ffmpegMediaFormat != null) {
            for (String key : ffmpegMediaFormat.getMap().keySet()) {
                nativeConfigure(key, ffmpegMediaFormat.getInteger(key));
            }
        }

    }

    public native void nativeConfigure(String key, int value);

    public native byte[] codec(byte[] src, int srcSize);

    public native void start();

    public native void release();
}
