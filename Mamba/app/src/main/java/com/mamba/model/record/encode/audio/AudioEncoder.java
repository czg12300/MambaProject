package com.mamba.model.record.encode.audio;

import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.encode.video.VideoEncoder;
import com.mamba.model.record.encode.video.VideoFrame;

/**
 * 音频编码器
 *
 * @author jake
 * @since 2017/7/6 下午5:13
 */

public interface AudioEncoder {

    void setCallback(Callback callback);

    void startEncode(AudioCodecParameters parameters);

    void offerRawFrame(AudioFrame data);

    void stopEncode();

    enum AudioEncoderType {
        FFMPEG, MEDIA_CODEC
    }

    interface Callback {
        void onStart();


        void onStop();
    }
}
