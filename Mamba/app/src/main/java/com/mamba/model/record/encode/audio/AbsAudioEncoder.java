package com.mamba.model.record.encode.audio;

import com.framework.ndk.media.SoundTouch;
import com.mamba.model.VLog;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 使用抽象的编码器实现
 *
 * @author jake
 * @since 2017/6/30 上午11:06
 */

public abstract class AbsAudioEncoder implements AudioEncoder, Runnable {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private Queue<AudioFrame> mTransList;
    private Callback callback;
    private SoundTouch mSoundTouch;

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
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!isRunning) {
                break;
            }

        }
        while (mTransList.size() > 0) {
            AudioFrame frame = mTransList.poll();
            if (frame != null && frame.data != null && frame.data.length > 0) {
                encode(frame);
            }
        }
        stopAndReleaseEncoder();
        isFinished = true;
        if (callback != null) {
            callback.onStop();
        }
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
            mSoundTouch = new SoundTouch();
            mSoundTouch.setupParams(0, parameters.channelCount, parameters.sampleRate, 2, 1, 1);
            VLog.d("ChangeSpeedTask   mSoundTouch.setupParams start");
//        mSoundTouch.setRate(1);
//            mSoundTouch.setRate();
//            mSoundTouch.setRateChange(parameters.speed);
        mSoundTouch.setTempo(1);
//            if (parameters.speed > 0) {
                mSoundTouch.setTempoChange(parameters.speed);
//            }
//            mSoundTouch.setPitchSemi(-1);
        }
        isFinished = false;
        new Thread(this).start();
    }

    @Override
    public void offerRawFrame(AudioFrame rawFrame) {
        if (rawFrame == null) {
            return;
        }
        if (mSoundTouch != null) {
            mSoundTouch.putBytes(rawFrame.data);
            int receiveSTSamples = 0;
            receiveSTSamples = mSoundTouch.getBytes(rawFrame.data);
            if (receiveSTSamples > 0) {
                byte[] result = new byte[receiveSTSamples];
                System.arraycopy(rawFrame.data, 0, result, 0, receiveSTSamples);
                rawFrame.data = result;
                rawFrame.size = result.length;
            } else {
                rawFrame = null;
            }
        }
        if (mTransList != null && rawFrame != null) {
            synchronized (mTransList) {
                mTransList.offer(rawFrame);
            }
        }

    }

    @Override
    public synchronized void stopEncode() {
        isRunning = false;
    }
}
