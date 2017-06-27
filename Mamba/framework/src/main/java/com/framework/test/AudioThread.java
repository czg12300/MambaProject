package com.framework.test;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.util.Log;

import com.framework.VideoClipJni;

import java.nio.ByteBuffer;

/**
 * 描述：
 *
 * @author walljiang
 * @since 2017/03/29 10:46
 */

public class AudioThread extends Thread {
    private final String TAG = "AudioThread";
    private static final String MIME_TYPE = "audio/mp4a-latm";
    private static final int SAMPLE_RATE = 44100;	// 44.1[KHz] is only setting guaranteed to be available on all devices.
    private static final int BIT_RATE = 57000;
    public static final int SAMPLES_PER_FRAME = 1024;	// AAC, bytes/frame/channel
    public static final int FRAMES_PER_BUFFER = 25; 	// AAC, frame/buffer/sec
    protected volatile boolean mRecordFlag = false;
    int channelLayout = AudioFormat.CHANNEL_IN_MONO;
    int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    private final String mAudioPath;
    private static final int[] AUDIO_SOURCES = new int[] {
            MediaRecorder.AudioSource.MIC,
            MediaRecorder.AudioSource.DEFAULT,
            MediaRecorder.AudioSource.CAMCORDER,
            MediaRecorder.AudioSource.VOICE_COMMUNICATION,
            MediaRecorder.AudioSource.VOICE_RECOGNITION,
    };

    public boolean getRecordFlag(){
        return mRecordFlag;
    }

    public void setRecordFlag(boolean recordFlag){
        this.mRecordFlag = recordFlag;
    }

    public AudioThread(String path){
        mAudioPath = path;
    }

    @Override
    public void run() {
        super.run();
        android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
        try {
            final int min_buffer_size = AudioRecord.getMinBufferSize(
                    SAMPLE_RATE,channelLayout,
                    audioFormat);
            int buffer_size = SAMPLES_PER_FRAME * FRAMES_PER_BUFFER;
            if (buffer_size < min_buffer_size)
                buffer_size = ((min_buffer_size / SAMPLES_PER_FRAME) + 1) * SAMPLES_PER_FRAME * 2;

            AudioRecord audioRecord = null;
            for (final int source : AUDIO_SOURCES) {
                try {
                    audioRecord = new AudioRecord(
                            source, SAMPLE_RATE,
                            channelLayout, audioFormat, buffer_size);
                    if (audioRecord.getState() != AudioRecord.STATE_INITIALIZED)
                        audioRecord = null;
                } catch (final Exception e) {
                    audioRecord = null;
                    e.printStackTrace();
                }
                if (audioRecord != null) break;
            }
            if (audioRecord != null) {
                try {
                    ByteBuffer buf = ByteBuffer.allocateDirect(SAMPLES_PER_FRAME);
                        int readBytes;
                        audioRecord.startRecording();
                        mRecordFlag = true;
                        try {
                            VideoClipJni.audioRecordStart(mAudioPath,audioRecord.getChannelCount(),BIT_RATE,SAMPLE_RATE);
                            for (; mRecordFlag;) {
                                buf.clear();
                                // read audio data from internal mic
                                readBytes = audioRecord.read(buf,SAMPLES_PER_FRAME);
                                int length = buf.remaining();
                                byte[] bytes = new byte[length];
                                buf.get(bytes,0,length);
                                if (readBytes > 0) {
                                    // set audio data to encoder
                                    VideoClipJni.audioRecording(bytes,length);
                                }
                            }
                        }catch (Exception e){
                          e.printStackTrace();
                        } finally {
                            VideoClipJni.audioRecordEnd();
                            audioRecord.stop();

                        }
                }catch (Exception e){
                    Log.e(TAG,e.getMessage());
                }
                finally {
                    audioRecord.release();
                }
            } else {
            }
        } catch (final Exception e) {
            Log.e(TAG, "AudioThread#run", e);
        }
        Log.v(TAG, "AudioThread:finished");
    }
}
