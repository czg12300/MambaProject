package com.mamba.model.record.encode.audio;

import android.media.AudioFormat;
import android.media.MediaRecorder;

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
    public int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    public int audioSource = MediaRecorder.AudioSource.MIC;
    public int channelLayout = AudioFormat.CHANNEL_IN_MONO;
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

        public Builder setSampleRate(int sampleRate) {
            parameters.sampleRate = sampleRate;
            return this;
        }

        public Builder setSpeed(float speed) {
            parameters.speed = speed;
            return this;
        }

        public Builder setAudioFormat(int audioFormat) {
            parameters.audioFormat = audioFormat;
            return this;
        }

        public Builder setAudioSource(int audioSource) {
            parameters.audioSource = audioSource;
            return this;
        }
        public Builder setChannelLayout(int channelLayout) {
            parameters.channelLayout = channelLayout;
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
