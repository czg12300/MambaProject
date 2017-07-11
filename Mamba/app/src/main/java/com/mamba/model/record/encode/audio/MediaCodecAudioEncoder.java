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
            mediaCodec.queueInputBuffer(inputBufferIndex, 0, frame.size, System.nanoTime(), 0);
        }
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
        while (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
            int len = bufferInfo.size + 7;
            byte[] outData = new byte[len];
            addADTStoPacket(outData, len);
            outputBuffer.get(outData, 7, bufferInfo.size);
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
            MediaFormat mediaFormat = MediaFormat.createAudioFormat(MINE_TYPE, parameters.sampleRate, parameters.channelCount);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, parameters.bitRate);
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

    /**
     * 给编码出的aac裸流添加adts头字段
     *
     * @param packet    要空出前7个字节，否则会搞乱数据
     * @param packetLen
     */
    private void addADTStoPacket(byte[] packet, int packetLen) {
        int profile = 2;  //AAC LC
        int freqIdx = 4;  //44.1KHz
        int chanCfg = 2;  //CPE
        packet[0] = (byte) 0xFF;
        packet[1] = (byte) 0xF9;
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }
}
