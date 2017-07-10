package com.mamba.model.record.encode.video;

/**
 * 数据帧回调
 *
 * @author jake
 * @since 2017/6/30 下午2:32
 */

public interface OnVideoFrameAvailableListener {

    void onFrameAvailable(VideoFrame frame);
}
