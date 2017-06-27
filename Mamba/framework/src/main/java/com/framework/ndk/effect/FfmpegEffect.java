package com.framework.ndk.effect;

/**
 * 使用ffmpeg处理音视频效果
 *
 * @author jake
 * @since 2017/6/1 下午4:45
 */

public class FfmpegEffect {
    /**
     * 正常
     */
    public static final int TYPE_NORMAL = 1;
    /**
     * 鬼畜
     */
    public static final int TYPE_REPEAT = 2;
    /**
     * 相对论
     */
    public static final int TYPE_RELATIVE = 3;

    public static void handleEffect(boolean isDealAudio,int effectType, String srcFile, String outFile, long rangStart, long rangeEnd, long effectStart, long effectEnd, OnEffectProgressListener listener) {
        nativeHandleEffect(isDealAudio,effectType, srcFile, outFile, rangStart, rangeEnd, effectStart, effectEnd, listener);
    }

    private final static native void nativeHandleEffect(boolean isDealAudio,int effectType, String srcFile, String outFile, long rangStart, long rangeEnd, long effectStart, long effectEnd, OnEffectProgressListener listener);
}
