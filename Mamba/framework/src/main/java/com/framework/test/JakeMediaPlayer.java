package com.framework.test;

import android.view.Surface;

/**
 * 自定义的播放器
 *
 * @author jake
 * @since 2017/1/19 上午10:37
 */

public class JakeMediaPlayer {
    static {
        System.loadLibrary("native-lib");
    }

    public JakeMediaPlayer() {
//        nativeInit();
    }

    public void setDataSource(String path) {
        setDataSourceNative(path);
    }

    public void prepare() {
        prepareNative();
    }

    public void start() {
        startNative();
    }

    public void pause() {
        prepareNative();
    }

    public void stop() {
        stopNative();
    }

    public void release() {
        releaseNative();
    }

    public void reset() {
        resetNative();
    }

    public void seekTo(long msec) {
        seekToNative(msec);
    }

    public void setSurface(Surface surface) {
        setSurfaceNative(surface);
    }

    public native void nativeInit();

    public native void setSurfaceNative(Surface surface);

    public native void setDataSourceNative(String path);

    public native void prepareNative();

    public native void startNative();

    public native void pauseNative();

    public native void stopNative();

    public native void releaseNative();

    public native void resetNative();

    public native void seekToNative(long msec);
}
