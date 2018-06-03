package com.mamba.codec;

public class VideoCodecParams {
    public enum CodecType {
        H264, H265
    }

    public int frameRate;
    public int bitRate;
    public CodecType codecType;
    public int width = 0;
    public int height = 0;
    //关键帧间隔时间 单位s
    public int keyIFrameInterval = 1;
    public String outFile;
    public long id;

    public static class VideoCodecParametersBuilder {
        private VideoCodecParams parameters;

        private VideoCodecParametersBuilder() {
            parameters = new VideoCodecParams();
        }

        public VideoCodecParametersBuilder setOutFile(String resultFile) {
            parameters.outFile = resultFile;
            return this;
        }

        public VideoCodecParametersBuilder setHeight(int height) {
            parameters.height = height;
            return this;
        }

        public VideoCodecParametersBuilder setKeyIFrameInterval(int minils) {
            parameters.keyIFrameInterval = minils;
            return this;
        }

        public VideoCodecParametersBuilder setWidth(int width) {
            parameters.width = width;
            return this;
        }

        public VideoCodecParametersBuilder setFrameRate(int frameRate) {
            parameters.frameRate = frameRate;
            return this;
        }


        public VideoCodecParametersBuilder setBitRate(int bitRate) {
            parameters.bitRate = bitRate;
            return this;
        }

        public VideoCodecParametersBuilder setId(long id) {
            parameters.id = id;
            return this;
        }

        public VideoCodecParametersBuilder setCodecType(CodecType codecType) {
            parameters.codecType = codecType;
            return this;
        }

        public VideoCodecParams build() {
            return parameters;
        }

        public static VideoCodecParametersBuilder create() {
            return new VideoCodecParametersBuilder();
        }
    }
}