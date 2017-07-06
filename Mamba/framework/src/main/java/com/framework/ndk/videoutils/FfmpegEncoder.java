package com.framework.ndk.videoutils;

/**
 * ffmpeg编码器
 *
 * @author jake
 * @since 2017/7/6 下午2:14
 */

public class FfmpegEncoder {

    public static native final int nativeStart(String file, int codecType, int keyIFrameInterval, int width, int height, int frameRate, int bitRate);

    public static native void nativeEncode(byte[] data, int width, int height);

    public static native final void nativeStop();
}
