package com.mamba.model.record.encode.audio;

/**
 * 音频采集监听
 *
 * @author jake
 * @since 2017/7/6 下午4:40
 */

public interface AudioRecordCallback {
    void onStart();

    void onFrameAvailable(byte[] data);

    void onStop();
}
