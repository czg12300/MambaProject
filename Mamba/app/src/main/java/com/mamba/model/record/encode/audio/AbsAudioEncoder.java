package com.mamba.model.record.encode.audio;

import com.mamba.model.VLog;
import com.mamba.model.record.encode.video.TransTask;
import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.encode.video.VideoEncoder;
import com.mamba.model.record.encode.video.VideoFrame;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 使用抽象的编码器实现
 *
 * @author jake
 * @since 2017/6/30 上午11:06
 */

public abstract class AbsAudioEncoder  implements AudioEncoder,Runnable, ChangeSpeedTask.Callback {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private volatile boolean isTransFinished = false;
    private Queue<AudioFrame> mTransList;
    private Callback callback;
    private ChangeSpeedTask changeSpeedTask;

    public AbsAudioEncoder() {
        mTransList = new LinkedBlockingQueue<>();

    }


    public synchronized boolean isRunning() {
        return isRunning;
    }

    @Override
    public void run() {
        if (callback != null) {
            callback.onStart();
        }
        while (true) {
            AudioFrame frame = null;
            if (mTransList != null) {
                synchronized (mTransList) {
                    frame = mTransList.poll();
                }
            }
            if (frame != null && frame.data != null && frame.data.length > 0) {
                encode(frame);
            } else {
                try {
                    Thread. sleep(30);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!isRunning && isTransFinished) {
                break;
            }
        }
        while (mTransList.size() > 0) {
            AudioFrame frame = mTransList.poll();
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

    protected abstract void encodeRawFrame(AudioFrame frame);

    private void encode(AudioFrame frame) {
        if (frame == null) {
            return;
        }
        encodeRawFrame(frame);
    }

    @Override
    public void onTrans(AudioFrame frame) {
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

    protected abstract boolean initAndStartEncode(AudioCodecParameters parameters);

    @Override
    public synchronized void startEncode(AudioCodecParameters parameters) {
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
        if (parameters.speed != 1) {
            changeSpeedTask = new ChangeSpeedTask();
            changeSpeedTask.setCallback(this);
            changeSpeedTask.startTask(parameters.speed);
            isTransFinished = false;
        }
        isFinished = false;
        new Thread(this).start();
    }

    @Override
    public void offerRawFrame(AudioFrame rawFrame) {
        if (changeSpeedTask != null) {
            changeSpeedTask.addRawData(rawFrame);
        } else {
            if (mTransList != null) {
                synchronized (mTransList) {
                    mTransList.offer(rawFrame);
                }
            }
        }
    }

    @Override
    public synchronized void stopEncode() {
        isRunning = false;
        if (changeSpeedTask != null) {
            changeSpeedTask.stopTask();
        }
    }
}
