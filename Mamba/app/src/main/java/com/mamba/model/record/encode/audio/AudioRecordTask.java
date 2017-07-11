package com.mamba.model.record.encode.audio;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.mamba.model.VLog;

import java.nio.ByteBuffer;

/**
 * 音频采集
 *
 * @author jake
 * @since 2017/7/6 下午4:38
 */

public class AudioRecordTask implements Runnable {
    private AudioRecordCallback audioFrameCallback;
    private AudioRecord mRecord;
    private volatile boolean isStop = false;
    private int SAMPLES_PER_FRAME = 1024;    // AAC, bytes/frame/channel
    private int FRAMES_PER_BUFFER = 25;    // AAC, frame/buffer/sec
    private AudioCodecParameters mAudioCodecParameters;

    public synchronized void startTask(AudioCodecParameters audioCodecParameters) {
        mAudioCodecParameters = audioCodecParameters;
        if (mAudioCodecParameters == null) {
            throw new NullPointerException("AudioCodec Parameters  is NUll");
        }
        isStop = false;
        new Thread(this).start();
    }

    public synchronized void stopTask() {
        isStop = true;
    }

    @Override
    public void run() {
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        prepare();
        if (mRecord != null) {
            mAudioCodecParameters.channelCount = mRecord.getChannelCount();
            ByteBuffer buf = ByteBuffer.allocateDirect(SAMPLES_PER_FRAME);
            mRecord.startRecording();
            if (audioFrameCallback != null) {
                audioFrameCallback.onStart(mAudioCodecParameters);
            }
            while (true) {
                buf.clear();
                // read audio data from internal mic
                int readBytes = mRecord.read(buf, SAMPLES_PER_FRAME);
                int length = buf.remaining();
                byte[] bytes = new byte[length];
                buf.get(bytes, 0, length);
                if (readBytes > 0) {
                    // set audio data to encoder
                    if (audioFrameCallback != null) {
                        audioFrameCallback.onFrameAvailable(bytes);
                    }
                }
                if (isStop) {
                    break;
                }
            }
            mRecord.release();
            mRecord = null;
            if (audioFrameCallback != null) {
                audioFrameCallback.onStop();
            }

        }
    }

    private void prepare() {
        final int min_buffer_size = AudioRecord.getMinBufferSize(mAudioCodecParameters.sampleRate, mAudioCodecParameters.channelLayout, mAudioCodecParameters.audioFormat);
        int buffer_size = SAMPLES_PER_FRAME * FRAMES_PER_BUFFER;
        if (buffer_size < min_buffer_size) {
            buffer_size = ((min_buffer_size / SAMPLES_PER_FRAME) + 1) * SAMPLES_PER_FRAME * 2;
        }
        try {
            mRecord = new AudioRecord(mAudioCodecParameters.audioSource, mAudioCodecParameters.sampleRate, mAudioCodecParameters.channelLayout, mAudioCodecParameters.audioFormat, buffer_size);
            VLog.d("mRecord  getChannelCount" + mRecord.getChannelCount());
        } catch (Exception e) {
            e.printStackTrace();
        } catch (Error e) {
            e.printStackTrace();
        }
    }

    public void setAudioFrameCallback(AudioRecordCallback audioFrameCallback) {
        this.audioFrameCallback = audioFrameCallback;
    }
}
