package com.mamba.model.record.encode.video;

import com.mamba.model.VLog;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 使用抽象的编码器实现
 *
 * @author jake
 * @since 2017/6/30 上午11:06
 */

public abstract class AbsVideoEncoder extends Thread implements VideoEncoder, TransTask.Callback {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private volatile boolean isTransFinished = false;
    private Queue<VideoFrame> mTransList;
    private Callback callback;
    private TransTask transTask;

    public AbsVideoEncoder() {
        mTransList = new LinkedBlockingQueue<>();
        transTask = new TransTask(getVideoEncoderType());
        transTask.setCallback(this);
    }

    protected abstract VideoEncoderType getVideoEncoderType();


    @Deprecated
    @Override
    public synchronized void start() {
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
            callback.onStop();
        }
        stopAndReleaseEncoder();
        isFinished = true;
    }

    protected abstract void stopAndReleaseEncoder();

    protected abstract void encodeRawFrame(VideoFrame frame);

    private void encode(VideoFrame frame) {
        if (frame == null || frame.writeTimes <= 0) {
            return;
        }
        for (int i = 0; i < frame.writeTimes; i++) {
            encodeRawFrame(frame);
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

    @Override
    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    protected abstract boolean initAndStartEncode(VideoCodecParameters parameters);

    @Override
    public synchronized void startEncode(VideoCodecParameters parameters) {
        if (parameters == null) {
            throw new NullPointerException("parameters is null,when start encode task use mediacodec");
        }
        boolean isInitSuccess = initAndStartEncode(parameters);
        if (!isInitSuccess) {
            VLog.e("initAndStartEncode  fail");
            return;
        }
        if (!isFinished) {
            mTransList.clear();
        }
        isRunning = true;
        transTask.setOutputSize(parameters.width, parameters.height);
        transTask.startTask();
        super.start();
        isFinished = false;
        isTransFinished = false;
    }

    @Override
    public void offerRawFrame(VideoFrame rawFrame) {
        if (transTask != null) {
            transTask.addRawData(rawFrame);
        }
    }

    @Override
    public synchronized void stopEncode() {
        isRunning = false;
        transTask.stopTask();
    }
}
