package com.mamba.model.record.encode1;

import com.framework.ndk.videoutils.PixelTransUtils;

import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * rgba转换为yuv的处理
 *
 * @author jake
 * @since 2017/6/30 上午10:23
 */

public class TransTask extends Thread {
    private volatile boolean isRunning = false;
    private volatile boolean isFinished = false;
    private Queue<VideoFrame> mRawList;
    private Callback callback;

    public TransTask() {
        mRawList = new LinkedBlockingQueue<>();
    }

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public synchronized void startTask() {
        if (!isFinished) {
            mRawList.clear();
        }
        isRunning = true;
        super.start();
        isFinished = false;
    }

    @Deprecated
    @Override
    public synchronized void start() {
    }

    public void addRawData(VideoFrame frame) {
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
        super.run();
        while (true) {
            VideoFrame frame = null;
            if (mRawList != null) {
                synchronized (mRawList) {
                    frame = mRawList.poll();
                }
            }
            if (frame != null && frame.data != null && frame.data.length > 0) {
                byte[] dataYuv = new byte[frame.width * frame.height * 3 / 2];
                PixelTransUtils.rgbaToYuv(frame.data, frame.width, frame.height, dataYuv);
                frame.data = dataYuv;
                if (callback != null) {
                    callback.onTrans(frame);
                }
            } else {
                try {
                    sleep(30);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!isRunning) {
                break;
            }
        }
        while (mRawList.size() > 0) {
            VideoFrame frame = mRawList.poll();
            if (frame != null && frame.data != null && frame.data.length > 0) {
                byte[] dataYuv = new byte[frame.width * frame.height * 3 / 2];
                PixelTransUtils.rgbaToYuv(frame.data, frame.width, frame.height, dataYuv);
                frame.data = dataYuv;
                if (callback != null) {
                    callback.onTrans(frame);
                }
            }
        }
        if (callback != null) {
            callback.onTransFinished();
        }
        isFinished = true;
    }

    public static interface Callback {
        void onTrans(VideoFrame frame);

        void onTransFinished();
    }
}
