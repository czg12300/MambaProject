package com.framework.ndk.effect;

/**
 * 效果处理进度监听
 *
 * @author jake
 * @since 2017/6/1 下午4:47
 */

public interface OnEffectProgressListener {
    void onProgress(int max, int progress);

    void onSuccess();

    void onFail();
}
