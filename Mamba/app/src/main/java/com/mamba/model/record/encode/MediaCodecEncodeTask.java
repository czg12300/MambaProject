package com.mamba.model.record.encode;

import android.media.MediaCodec;
import android.media.MediaFormat;

import com.mamba.model.VLog;

import java.nio.ByteBuffer;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 使用media codec编码
 *
 * @author jake
 * @since 2017/6/30 上午11:06
 */

public class MediaCodecEncodeTask extends Thread implements TransTask.Callback {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private volatile boolean isTransFinished = false;
    private Queue<VideoFrame> mTransList;
    private Callback callback;
    private MediaCodec mEncoder;
    private TransTask transTask;
    private long frameIndex = 0;
    private long frameRate = 0;

    public MediaCodecEncodeTask() {
        mTransList = new LinkedBlockingQueue<>();
        transTask = new TransTask();
        transTask.setCallback(this);
    }


    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public synchronized void startTask(MediaCodec encoder) {
        if (encoder == null) {
            throw new NullPointerException("encoder is null ,when start encode task");
        }
        frameIndex = 0;
        mEncoder = encoder;
        frameRate = encoder.getOutputFormat().getInteger(MediaFormat.KEY_FRAME_RATE);
        VLog.d("frameRate    frameRate  " + frameRate);
        if (!isFinished) {
            mTransList.clear();
        }
        isRunning = true;
        transTask.setOutputSize(encoder.getOutputFormat().getInteger(MediaFormat.KEY_WIDTH), encoder.getOutputFormat().getInteger(MediaFormat.KEY_HEIGHT));
        transTask.startTask();
        super.start();
        isFinished = false;
        isTransFinished = false;
    }

    public void addRawData(VideoFrame frame) {
        if (transTask != null) {
            transTask.addRawData(frame);
        }
    }

    @Deprecated
    @Override
    public synchronized void start() {
    }

    public synchronized void stopTask() {
        isRunning = false;
        transTask.stopTask();
    }

    public synchronized boolean isRunning() {
        return isRunning;
    }

    @Override
    public void run() {
        super.run();
        if (callback != null) {
            callback.onStart();
        }
        while (true) {
            VideoFrame frame = null;
            if (mTransList != null) {
                synchronized (mTransList) {
                    frame = mTransList.poll();
                }
            }
            if (frame != null && frame.data != null && frame.data.length > 0) {
                encode(frame);
            } else {
                try {
                    sleep(30);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!isRunning && isTransFinished) {
                break;
            }
        }
        while (mTransList.size() > 0) {
            VideoFrame frame = mTransList.poll();
            if (frame != null && frame.data != null && frame.data.length > 0) {
                encode(frame);
            }
        }
        if (callback != null) {
            callback.onFinish();
        }
        isFinished = true;
    }

    private void encode(VideoFrame frame) {
        if (mEncoder == null || frame == null || frame.writeTimes <= 0) {
            return;
        }
        for (int i = 0; i < frame.writeTimes; i++) {

            //输入
            ByteBuffer[] inputBuffers = mEncoder.getInputBuffers();
            ByteBuffer[] outputBuffers = mEncoder.getOutputBuffers();
            int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);

            VLog.d("MediaVideoCodec  encode(byte[] data, int width, int height) inputBufferIndex=" + i);
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
                if (callback != null) {
                    callback.onEncode(outData);
                }
                mEncoder.releaseOutputBuffer(outputBufferIndex, false);
                outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 1100);
            }
        }
    }

    private long computePresentationTime(long frameIndex) {
        return frameIndex == 0 ? 0 : frameIndex * 1000000 / frameRate ;
    }

    @Override
    public void onTrans(VideoFrame frame) {
        if (mTransList != null) {
            synchronized (mTransList) {
                mTransList.offer(frame);
            }
        }
    }

    @Override
    public void onTransFinished() {
        isTransFinished = true;
    }

    public static interface Callback {
        void onStart();

        void onEncode(byte[] data);

        void onFinish();
    }
}
