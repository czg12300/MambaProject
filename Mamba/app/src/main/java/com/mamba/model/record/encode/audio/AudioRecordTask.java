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
    private static final int SAMPLE_RATE = 44100;    // 44.1[KHz] is only setting guaranteed to be available on all devices.
    private static final int SAMPLES_PER_FRAME = 1024;    // AAC, bytes/frame/channel
    private static final int FRAMES_PER_BUFFER = 25;    // AAC, frame/buffer/sec
    int channelLayout = AudioFormat.CHANNEL_IN_MONO;
    int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    private static final int[] AUDIO_SOURCES = new int[]{
            MediaRecorder.AudioSource.MIC,
            MediaRecorder.AudioSource.DEFAULT,
            MediaRecorder.AudioSource.CAMCORDER,
            MediaRecorder.AudioSource.VOICE_COMMUNICATION,
            MediaRecorder.AudioSource.VOICE_RECOGNITION,
    };

    public AudioRecordTask() {
    }


    public synchronized void startTask() {
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
            ByteBuffer buf = ByteBuffer.allocateDirect(SAMPLES_PER_FRAME);
            mRecord.startRecording();
            if (audioFrameCallback != null) {
                audioFrameCallback.onStart();
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
        try {
            final int min_buffer_size = AudioRecord.getMinBufferSize(
                    SAMPLE_RATE, channelLayout,
                    audioFormat);
            int buffer_size = SAMPLES_PER_FRAME * FRAMES_PER_BUFFER;
            if (buffer_size < min_buffer_size) {
                buffer_size = ((min_buffer_size / SAMPLES_PER_FRAME) + 1) * SAMPLES_PER_FRAME * 2;
            }
            for (final int source : AUDIO_SOURCES) {
                try {
                    mRecord = new AudioRecord(
                            source, SAMPLE_RATE,
                            channelLayout, audioFormat, buffer_size);
                    VLog.d("mRecord  getChannelCount"+mRecord.getChannelCount());
                    if (mRecord.getState() != AudioRecord.STATE_INITIALIZED) {
                        mRecord = null;
                    }
                } catch (final Exception e) {
                    mRecord = null;
                    e.printStackTrace();
                }
                if (mRecord != null) {
                    break;
                }
            }
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
