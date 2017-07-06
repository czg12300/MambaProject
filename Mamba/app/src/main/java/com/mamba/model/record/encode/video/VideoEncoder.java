package com.mamba.model.record.encode.video;

/**
 * 视频编码器
 *
 * @author jake
 * @since 2017/7/6 上午10:08
 */

public interface VideoEncoder {

    void setCallback(Callback callback);

    void startEncode(VideoCodecParameters parameters);

    void offerRawFrame(VideoFrame rawFrame);

    void stopEncode();

    enum VideoEncoderType {
        FFMPEG, MEDIA_CODEC
    }

    interface Callback {
        void onStart();


        void onStop();
    }
}
