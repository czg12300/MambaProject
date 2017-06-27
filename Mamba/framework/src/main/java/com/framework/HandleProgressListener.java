package com.framework;

/**
 * 音视频处理的进度监听
 *
 * @author jake
 * @since 2017/3/31 下午3:00
 */

public interface HandleProgressListener {
    void onHandle(int progress, int total);

    void onSuccess(String outFile);
}
