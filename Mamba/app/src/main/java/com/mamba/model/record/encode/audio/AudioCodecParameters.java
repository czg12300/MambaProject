package com.mamba.model.record.encode.audio;

import com.mamba.model.record.encode.video.VideoCodecParameters;

/**
 * 音频录制参数
 *
 * @author jake
 * @since 2017/7/6 上午11:42
 */

public class AudioCodecParameters {
    public int channelCount;
    public int bitRate;
    public int sampleRate;
    public String outFile;
    public float speed = 1;

    public static class Builder {
        private AudioCodecParameters parameters;

        private Builder() {
            parameters = new AudioCodecParameters();
        }

        public Builder setOutFile(String resultFile) {
            parameters.outFile = resultFile;
            return this;
        }

        public Builder setChannelCount(int channelCount) {
            parameters.channelCount = channelCount;
            return this;
        }

        public Builder setSampleRate(int sampleRate) {
            parameters.sampleRate = sampleRate;
            return this;
        }

        public Builder setSpeed(int speed) {
            parameters.speed = speed;
            return this;
        }

        public Builder setBitRate(int bitRate) {
            parameters.bitRate = bitRate;
            return this;
        }

        public AudioCodecParameters build() {
            return parameters;
        }

        public static Builder create() {
            return new Builder();
        }
    }
}
