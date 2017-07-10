package com.mamba.model.record.encode.audio;

import android.media.MediaCodec;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.media.MediaFormat;
import android.media.audiofx.Equalizer;

import java.io.DataOutputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author jake
 * @since 2017/7/6 下午5:16
 */

public class MediaCodecAudioEncoder extends AbsAudioEncoder {
    private final static String MINE_TYPE = "audio/mp4a-latm";
    private MediaCodec mediaCodec;
    private DataOutputStream mOutput;

    @Override
    protected void stopAndReleaseEncoder() {
        mediaCodec.stop();
        mediaCodec.release();
        if (mOutput != null) {
            try {
                mOutput.flush();
                mOutput.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
    }

    @Override
    protected void encodeRawFrame(AudioFrame frame) {
        if (frame == null || frame.data == null) {
            return;
        }
        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
        int inputBufferIndex = mediaCodec.dequeueInputBuffer(-1);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();
            inputBuffer.put(frame.data);
            mediaCodec.queueInputBuffer(inputBufferIndex, 0, frame.size, System.nanoTime() / 1000, 0);
        }
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
        while (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
            byte[] outData = new byte[bufferInfo.size];
            outputBuffer.get(outData);
            try {
                mOutput.write(outData);
            } catch (Exception e) {
                e.printStackTrace();
            } catch (Error e) {
                e.printStackTrace();
            }
            mediaCodec.releaseOutputBuffer(outputBufferIndex, false);
            outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
        }
    }

    @Override
    protected boolean initAndStartEncode(AudioCodecParameters parameters) {
        boolean isSuccess = false;
        try {
            mediaCodec = MediaCodec.createEncoderByType(MINE_TYPE);
            MediaFormat mediaFormat = MediaFormat.createAudioFormat(MINE_TYPE, 44100, 1);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, parameters.bitRate);
            mediaFormat.setInteger(MediaFormat.KEY_SAMPLE_RATE, parameters.sampleRate);
            mediaFormat.setInteger(MediaFormat.KEY_CHANNEL_COUNT, parameters.channelCount);
            mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mediaCodec.start();
        } catch (IOException e) {
            e.printStackTrace();
            return isSuccess;
        }
        try {
            mOutput = new DataOutputStream(new FileOutputStream(parameters.outFile));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return isSuccess;
        }
        isSuccess = true;
        return isSuccess;
    }
}