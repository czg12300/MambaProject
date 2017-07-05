package com.mamba.model.record.encode;

import com.framework.ndk.videoutils.PixelTransUtils;
import com.mamba.model.VLog;

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
    private int outWidth;
    private int outHeight;

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

    public void setOutputSize(int outWidth, int outHeight) {
        this.outWidth = outWidth;
        this.outHeight = outHeight;
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
                trans(frame);
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
                trans(frame);
            }
        }
        if (callback != null) {
            callback.onTransFinished();
        }
        isFinished = true;
    }

    private void trans(VideoFrame frame) {
        byte[] dataYuv = new byte[outWidth * outHeight * 3 / 2];
        long time = System.currentTimeMillis();
        PixelTransUtils.nv21ToYv12(frame.data, frame.width, frame.height, dataYuv, outWidth, outHeight, frame.rotate);
        long timea = System.currentTimeMillis();
        VLog.d("trans   time=" + (timea - time));
        frame.data = dataYuv;
        if (callback != null) {
            callback.onTrans(frame);
        }
    }

    public static interface Callback {
        void onTrans(VideoFrame frame);

        void onTransFinished();
    }
}
