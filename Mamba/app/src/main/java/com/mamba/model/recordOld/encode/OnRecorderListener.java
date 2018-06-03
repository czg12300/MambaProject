package com.mamba.model.recordOld.encode;

/**
 * 录制监听
 *
 * @author jake
 * @since 2017/8/8 下午6:20
 */

public interface OnRecorderListener {
    int TYPE_VIDEO=0x01;
    int TYPE_AUDIO=0x02;
    void onStart(long id, int type);

    void onFail(long id, int type);

    void onSuccess(long id, int type);
}
