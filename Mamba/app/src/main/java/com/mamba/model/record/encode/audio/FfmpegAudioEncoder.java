package com.mamba.model.record.encode.audio;

import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.encode.video.VideoEncoder;

/**
 * @author jake
 * @since 2017/7/6 下午5:17
 */

public class FfmpegAudioEncoder extends AbsAudioEncoder {
    @Override
    protected void stopAndReleaseEncoder() {

    }

    @Override
    protected void encodeRawFrame(AudioFrame frame) {

    }

    @Override
    protected boolean initAndStartEncode(AudioCodecParameters parameters) {
        return false;
    }
}
