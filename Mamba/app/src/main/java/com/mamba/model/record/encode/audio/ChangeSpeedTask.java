package com.mamba.model.record.encode.audio;

import com.framework.ndk.videoutils.SoundTouchUtils;
import com.mamba.model.VLog;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 改变速率
 *
 * @author jake
 * @since 2017/7/10 下午3:37
 */

public class ChangeSpeedTask implements Runnable {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private Queue<AudioFrame> mRawList;
    private Callback callback;
    private SoundTouchUtils mSoundTouchUtils;

    public ChangeSpeedTask() {
        mRawList = new LinkedBlockingQueue<>();
        mSoundTouchUtils = new SoundTouchUtils();
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public synchronized void startTask(float speed) {
        if (!isFinished) {
            mRawList.clear();
        }
        isRunning = true;
        mSoundTouchUtils.init();
        mSoundTouchUtils.setRateChange(speed);
        isFinished = false;
        new Thread(this).start();
    }


    public void addRawData(AudioFrame frame) {
        if (!isRunning || frame == null) {
            return;
        }
        if (frame != null && mRawList != null) {
            synchronized (mRawList) {
                mRawList.offer(frame);
            }
        }
    }

    public synchronized void stopTask() {
        isRunning = false;
    }

    public synchronized boolean isRunning() {
        return isRunning;
    }

    @Override
    public void run() {
        while (true) {
            AudioFrame frame = null;
            if (mRawList != null) {
                synchronized (mRawList) {
                    frame = mRawList.poll();
                }
            }
            if (frame != null && frame.data != null && frame.size > 0) {
                trans(frame);
            } else {
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!isRunning) {
                break;
            }
        }
        while (mRawList.size() > 0) {
            AudioFrame frame = mRawList.poll();
            if (frame != null && frame.data != null && frame.size > 0) {
                trans(frame);
            }
        }
        mSoundTouchUtils.release();
        if (callback != null) {
            callback.onTransFinished();
        }
        isFinished = true;
    }

    private void trans(AudioFrame frame) {
        long time = System.currentTimeMillis();
        mSoundTouchUtils.putSampleByBytes(frame.data, frame.size);
        byte[] result = new byte[frame.size];
        mSoundTouchUtils.getSampleByBytes(result, frame.size);
        frame.data = result;
        long timea = System.currentTimeMillis();
        VLog.d("trans ChangeSpeedTask  time=" + (timea - time));
        if (callback != null) {
            callback.onTrans(frame);
        }
    }

    public static interface Callback {
        void onTrans(AudioFrame frame);

        void onTransFinished();
    }
}
