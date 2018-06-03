package com.jake.mamba.utils;

import android.util.Log;

/**
 * @author jake
 * @since 2018/4/15 下午11:38
 */

public class FpsUtil {
    static long lastTimes = 0;
    static int count = 0;

    /**
     * 打印fps
     */
    public static void printFps() {
        long now = System.currentTimeMillis();
        if (lastTimes > 0) {
            if (now - lastTimes >= 1000) {
                Log.d("tag", "CameraRenderer fps:" + count);
                lastTimes = now;
                count = 0;
            } else {
                count++;
            }
        } else {
            lastTimes = now;
        }
    }
}
