package com.mamba.model.record.encode.video;

import com.framework.ndk.videoutils.FfmpegEncoder;
import com.mamba.model.VLog;

/**
 * 基于ffmpeg的视频编码器实现
 *
 * @author jake
 * @since 2017/7/6 上午10:25
 */

public class FfmpegVideoEncoder extends AbsVideoEncoder {
    public FfmpegVideoEncoder() {
        super();
    }

    @Override
    protected VideoEncoderType getVideoEncoderType() {
        return VideoEncoderType.FFMPEG;
    }

    @Override
    protected void stopAndReleaseEncoder() {
        FfmpegEncoder.nativeStop();
    }

    @Override
    protected void encodeRawFrame(VideoFrame frame) {
        if (frame != null) {
            FfmpegEncoder.nativeEncode(frame.data, frame.width, frame.height);
        }
    }

    @Override
    protected boolean initAndStartEncode(VideoCodecParameters parameters) {
        return FfmpegEncoder.nativeStart(parameters.outFile, transCodecType(parameters.codecType), parameters.keyIFrameInterval, parameters.width, parameters.height, parameters.frameRate, parameters.bitRate) >= 0;
    }

    private int transCodecType(VideoCodecParameters.CodecType codecType) {
        int result = 1;
        switch (codecType) {
            case H264:
                result = 1;
                break;
            case H265:
                result = 2;
                break;
        }
        VLog.d("ndk-log", "result =" + result);
        return result;
    }

}
