package com.mamba.model.record.encode.video;

/**
 * 视频的编码的参数
 *
 * @author jake
 * @since 2017/5/2 下午5:47
 */

public class VideoCodecParameters {
    public static enum CodecType {
        H264, H265
    }

    public int frameRate;
    public int speedFrameRate;
    public int bitRate;
    public CodecType codecType;
    public int width = 0;
    public int height = 0;
    //关键帧间隔时间 单位s
    public int keyIFrameInterval = 1;
    public String outFile;

    public static class Builder {
        private VideoCodecParameters parameters;

        private Builder() {
            parameters = new VideoCodecParameters();
        }

        public Builder setOutFile(String resultFile) {
            parameters.outFile = resultFile;
            return this;
        }

        public Builder setHeight(int height) {
            parameters.height = height;
            return this;
        }

        public Builder setKeyIFrameInterval(int minils) {
            parameters.keyIFrameInterval = minils;
            return this;
        }

        public Builder setWidth(int width) {
            parameters.width = width;
            return this;
        }

        public Builder setFrameRate(int frameRate) {
            parameters.frameRate = frameRate;
            return this;
        }

        public Builder setSpeedFrameRate(int speedFrameRate) {
            parameters.speedFrameRate = speedFrameRate;
            return this;
        }

        public Builder setBitRate(int bitRate) {
            parameters.bitRate = bitRate;
            return this;
        }

        public Builder setCodecType(CodecType codecType) {
            parameters.codecType = codecType;
            return this;
        }

        public VideoCodecParameters build() {
            return parameters;
        }

        public static Builder create() {
            return new Builder();
        }
    }
}