package com.mamba.model.record.encode1;

import android.media.MediaCodec;

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

    public MediaCodecEncodeTask() {
        mTransList = new LinkedBlockingQueue<>();
        transTask = new TransTask();
        transTask.setCallback(this);
    }


    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public synchronized void startTask(MediaCodec encoder) {
        mEncoder = encoder;
        if (!isFinished) {
            mTransList.clear();
        }
        isRunning = true;
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
        if (mEncoder == null) {
            return;
        }
        //输入
        ByteBuffer[] inputBuffers = mEncoder.getInputBuffers();
        int inputBufferIndex = mEncoder.dequeueInputBuffer(-1);

        VLog.d("MediaVideoCodec  encode(byte[] data, int width, int height) inputBufferIndex=" + inputBufferIndex);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inputBufferIndex];
            inputBuffer.clear();
            inputBuffer.put(frame.data);
//            mMediaCodec.queueInputBuffer(inputBufferIndex, 0, data.length,  0,0);
            mEncoder.queueInputBuffer(inputBufferIndex, 0, frame.data.length, System.nanoTime() / 1000, MediaCodec.BUFFER_FLAG_CODEC_CONFIG);
        }
//输出
        ByteBuffer[] outputBuffers = mEncoder.getOutputBuffers();
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
        while (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
            byte[] outData = new byte[bufferInfo.size];
            outputBuffer.get(outData);
            if (callback != null) {
                callback.onEncode(outData);
            }
            mEncoder.releaseOutputBuffer(outputBufferIndex, false);
            outputBufferIndex = mEncoder.dequeueOutputBuffer(bufferInfo, 0);
        }
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
