package com.mamba.model.record.encode.video;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.support.annotation.NonNull;

import com.framework.utils.FileUtil;
import com.mamba.model.VLog;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 使用media codec编码
 *
 * @author jake
 * @since 2017/6/30 上午11:06
 */

public class MediaCodecEncoder extends AbsVideoEncoder {
    private MediaCodec mEncoder;
    private long frameIndex = 0;
    private long frameRate = 0;
    private DataOutputStream mOutput;

    public MediaCodecEncoder() {
        super();
    }

    @Override
    protected VideoEncoderType getVideoEncoderType() {
        return VideoEncoderType.MEDIA_CODEC;
    }

    private String transCodecType(VideoCodecParameters parameters) {
        String result = MediaFormat.MIMETYPE_VIDEO_AVC;
        if (parameters != null) {
            switch (parameters.codecType) {
                case H264:
                    result = MediaFormat.MIMETYPE_VIDEO_AVC;
                    break;
                case H265:
                    result = MediaFormat.MIMETYPE_VIDEO_HEVC;
                    break;
            }
        }
        return result;
    }

    @NonNull
    private MediaFormat getMediaFormat(VideoCodecParameters parameters) {
        MediaFormat format = MediaFormat.createVideoFormat(transCodecType(parameters), parameters.width, parameters.height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        format.setInteger(MediaFormat.KEY_BIT_RATE, parameters.bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, parameters.frameRate);
        format.setInteger(MediaFormat.KEY_CAPTURE_RATE, parameters.frameRate);
//        format.setInteger(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, 40);
        if (parameters.keyIFrameInterval > 0) {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, parameters.keyIFrameInterval);
        } else {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        }
        return format;
    }


    @Override
    protected void stopAndReleaseEncoder() {
        mEncoder.stop();
        mEncoder.release();
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
    protected void encodeRawFrame(VideoFrame frame) {
        //输入
        ByteBuffer[] inputBuffers = mEncoder.getInputBuffers();
        ByteBuffer[] outputBuffers = mEncoder.getOutputBuffers();
        int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);
        VLog.d("MediaVideoCodec  encode(byte[] data, int width, int height) inputBufferIndex=");
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();
            inputBuffer.put(frame.data);
            long timestamp = computePresentationTime(frameIndex++);
            VLog.d("MediaVideoCodec  timestamp " + timestamp);
            mEncoder.queueInputBuffer(inputBufferIndex, 0, frame.data.length, timestamp, 0);
        }
//输出
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 1100);
        while (outputBufferIndex >= 0) {
            VLog.d("while (outputBufferIndex >= 0)");
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
            mEncoder.releaseOutputBuffer(outputBufferIndex, false);
            outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 1100);
        }
    }

    private long computePresentationTime(long frameIndex) {
        return frameIndex == 0 ? 0 : frameIndex * 1000000 / frameRate;
    }

    @Override
    protected boolean initAndStartEncode(VideoCodecParameters parameters) {
        boolean isInitSuccess = false;
        try {
            mEncoder = mEncoder.createEncoderByType(transCodecType(parameters));
        } catch (IOException e) {
            e.printStackTrace();
            return isInitSuccess;
        }
        mEncoder.configure(getMediaFormat(parameters), null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        mEncoder.start();
        File h264 = new File(parameters.outFile);
        FileUtil.createFile(h264.getAbsolutePath(), true);
        try {
            mOutput = new DataOutputStream(new FileOutputStream(h264));
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            return isInitSuccess;
        }
        frameIndex = 0;
        frameRate = parameters.frameRate;

        VLog.d("frameRate    frameRate  " + frameRate);
        isInitSuccess = true;
        return isInitSuccess;
    }
}
