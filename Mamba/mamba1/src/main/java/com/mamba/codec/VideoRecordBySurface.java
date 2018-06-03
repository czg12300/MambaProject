package com.mamba.codec;


import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.opengl.EGLContext;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.mamba.OnRecorderListener;
import com.mamba.renderer.CameraRenderer;
import com.mamba.renderer.gpuimage.filter.GPUImageFilter;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingDeque;

public class VideoRecordBySurface {
    private static final int TIME_OUT = 1000;
    private OnRecorderListener mOnRecorderListener;
    private MediaCodec mMediaCodec;
    private VideoCodecParams mParams;
    private PrepareParams mPrepareParams;
    private Queue<RawFrame> mRawFrameList;
    private OutputStream mOutput;

    private int mFrameIndex = 0;
    private volatile float mSpeed = 1.0f;
    private volatile boolean mIsRecord = false;
    private boolean mNeedInitCodec = false;
    private VideoCodecRenderer mVideoCodecRenderer;
    private long mLastFrameTimestamp = 0;
    private long mFirstFrameTimestamp = 0;

    public VideoRecordBySurface() {
        mVideoCodecRenderer = new VideoCodecRenderer();
    }

    public void setOnRecorderListener(OnRecorderListener onRecorderListener) {
        this.mOnRecorderListener = onRecorderListener;
    }

    public void start(VideoCodecParams params) {
        mParams = params;
        mIsRecord = true;
        mNeedInitCodec = true;
    }

    public void setSpeed(float speed) {
        mSpeed = 1.0f / speed;
        Log.d("tag", "timestamp mSpeed " + mSpeed);
    }
public void setFilter(GPUImageFilter filter){
        mVideoCodecRenderer.setFilter(filter);
}
    public void stop() {
        mIsRecord = false;
    }


    private String transCodecType(VideoCodecParams.CodecType codecType) {
        String result = MediaFormat.MIMETYPE_VIDEO_AVC;
        switch (codecType) {
            case H264:
                result = MediaFormat.MIMETYPE_VIDEO_AVC;
                break;
            case H265:
                result = MediaFormat.MIMETYPE_VIDEO_HEVC;
                break;
        }
        return result;
    }

    private void encode() {
        while (true) {
            RawFrame frame = pollRawFrame();
            if (frame != null) {
                handFrameUpdate(frame);
            } else {
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!mIsRecord) {
                break;
            }
        }
//        RawFrame frame = pollRawFrame();
//        while (frame != null) {
//            handFrameUpdate(frame);
//            frame = pollRawFrame();
//        }
        mMediaCodec.signalEndOfInputStream();
    }

    private MediaCodec.Callback mMediaCodecCallback = new MediaCodec.Callback() {
        private byte[] configByte;

        @Override
        public void onInputBufferAvailable(@NonNull MediaCodec codec, int index) {

        }

        @Override
        public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo bufferInfo) {
//            int status = codec.dequeueOutputBuffer(bufferInfo, 10000);
//                if (status >= 0) {
                ByteBuffer outputBuffer = codec.getOutputBuffer(index);
                byte[] outData = new byte[bufferInfo.size];
                outputBuffer.get(outData);
                try {
                    if (bufferInfo.flags == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {
                        configByte = new byte[bufferInfo.size];
                        configByte = outData;
                    } else if (bufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME) {
                        byte[] keyframe = new byte[bufferInfo.size + configByte.length];
                        System.arraycopy(configByte, 0, keyframe, 0, configByte.length);
                        System.arraycopy(outData, 0, keyframe, configByte.length, outData.length);
                        mOutput.write(keyframe, 0, keyframe.length);
                    } else {
                        mOutput.write(outData, 0, outData.length);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                } catch (Error e) {
                    e.printStackTrace();
                }
                codec.releaseOutputBuffer(index, false);
//            }
        }

        @Override
        public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException e) {

        }

        @Override
        public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format) {
        }
    };

    private void handFrameUpdate(RawFrame rawFrame) {
        int count = calculateEncodeTimes((long) (rawFrame.timestamp / 1000000.0f));
        Log.d("tag", "count =" + count);
        for (int i = 0; i < count; i++) {
            mVideoCodecRenderer.drawFrame(rawFrame.transform, computePresentationTime());
        }

    }

    private int calculateEncodeTimes(long timestamp) {
        Log.d("tag", "timestamp =" + timestamp);
        int result = 0;
        if (mLastFrameTimestamp > 0) {
            long preTimestampSpit = (long) (1000.0f / (mParams.frameRate * mSpeed));
            long preTimestamp = mLastFrameTimestamp + preTimestampSpit;
            if (timestamp >= preTimestamp) {
                int timestampTimes = (int) (((timestamp - mLastFrameTimestamp) * 1.0f) / preTimestampSpit);
                result = timestampTimes;
                mLastFrameTimestamp = mLastFrameTimestamp + timestampTimes * preTimestampSpit;
            }
        } else {
            result = 1;
            mLastFrameTimestamp = timestamp;
        }
        return result;
    }

    private long computePresentationTime() {
        long timestamp = (long) (132 + mFrameIndex * 1000000000.0f / mParams.frameRate);
        mFrameIndex++;
        Log.d("tag", "mParams.frameRate =" + mParams.frameRate + "   timestamp =" + String.valueOf(timestamp) + "  mFrameIndex =" + mFrameIndex + " mSpeed=" + mSpeed);
        return timestamp;
    }


    private CameraRenderer.OnSurfaceRendererListener mOnSurfaceRendererListener = new CameraRenderer.OnSurfaceRendererListener() {
        @Override
        public void onRenderer(EGLContext sharedContext, int textureId, int rotation, float[] transform, long timestamp, int width, int height) {
            if (mIsRecord) {
                if (mNeedInitCodec) {
                    mPrepareParams = new PrepareParams(sharedContext, textureId, rotation, width, height);
                    mRawFrameList = new LinkedBlockingDeque<>();
                    mNeedInitCodec = false;
                    new Thread(mCodecRunnable).start();
                    mFirstFrameTimestamp = timestamp;
                }
                if (timestamp == 0) {
                    // Seeing this after device is toggled off/on with power button.  The
                    // first frame back has a zero timestamp.
                    // MPEG4Writer thinks this is cause to abort() in native code, so it's very
                    // important that we just ignore the frame.
                    return;
                }
                synchronized (mRawFrameList) {
                    mRawFrameList.offer(new RawFrame(transform, timestamp - mFirstFrameTimestamp));
                }
            }
        }
    };
    private Runnable mCodecRunnable = new Runnable() {
        private volatile boolean mIsPrepare = false;

        @Override
        public void run() {
            mIsPrepare = false;
            try {
                prepare();
                mIsPrepare = true;
            } catch (IOException e) {
                e.printStackTrace();
            }
            if (mIsPrepare) {
                if (mOnRecorderListener != null) {
                    mOnRecorderListener.onStart(mParams.id, OnRecorderListener.TYPE_VIDEO);
                }
                encode();
            }
            release();
            if (mOnRecorderListener != null) {
                boolean isSuccess = mIsPrepare;
                if (!TextUtils.isEmpty(mParams.outFile)) {
                    File file = new File(mParams.outFile);
                    if (file.exists() && file.isFile()) {
                        if (file.length() > 50) {
                            isSuccess = mIsPrepare && true;
                        } else {
                            file.delete();
                        }
                    }
                } else {
                    isSuccess = false;
                }
                if (isSuccess) {
                    mOnRecorderListener.onSuccess(mParams.id, OnRecorderListener.TYPE_VIDEO, mParams.outFile);
                } else {
                    mOnRecorderListener.onFail(mParams.id, OnRecorderListener.TYPE_VIDEO);
                }
            }
        }
    };

    private void prepare() throws IOException {
        String mime = transCodecType(mParams.codecType);
        MediaFormat format = MediaFormat.createVideoFormat(mime, mParams.width, mParams.height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, mParams.bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, mParams.frameRate);
        if (mParams.keyIFrameInterval > 0) {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, mParams.keyIFrameInterval);
        } else {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        }
        mMediaCodec = mMediaCodec.createEncoderByType(mime);
        mMediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        Surface surface = mMediaCodec.createInputSurface();
        mMediaCodec.setCallback(mMediaCodecCallback);
        mMediaCodec.start();
        if (mPrepareParams != null) {
            mVideoCodecRenderer.prepare(mPrepareParams.sharedContext, surface, mPrepareParams.rotation, mPrepareParams.textureId, mPrepareParams.width, mPrepareParams.height, mParams.width, mParams.height);
        }
        if (TextUtils.isEmpty(mParams.outFile)) {
            throw new FileNotFoundException("outFile is null,please set out file");
        }
        File file = new File(mParams.outFile);
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdirs();
        }
        if (file.exists()) {
            file.delete();
        }
        file.createNewFile();
        mOutput = new DataOutputStream(new FileOutputStream(mParams.outFile));
        mFrameIndex = 0;
        mLastFrameTimestamp = 0;
    }

    private void release() {
        mIsRecord = false;
        if (mVideoCodecRenderer != null) {
            mVideoCodecRenderer.release();
        }
        if (mMediaCodec != null) {
            mMediaCodec.stop();
            mMediaCodec.release();
            mMediaCodec = null;
        }
        if (mOutput != null) {
            try {
                mOutput.flush();
                mOutput.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mOutput = null;
        }
        if (mRawFrameList != null) {
            mRawFrameList.clear();
            mRawFrameList = null;
        }

        mPrepareParams = null;
    }

    public CameraRenderer.OnSurfaceRendererListener getOnSurfaceRendererListener() {
        return mOnSurfaceRendererListener;
    }


    private RawFrame pollRawFrame() {
        RawFrame frame = null;
        if (mRawFrameList != null) {
            int len = mRawFrameList.size();
            if (len > 0) {
                synchronized (mRawFrameList) {
                    frame = mRawFrameList.poll();
                }
            }
        }
        return frame;
    }

    private static class RawFrame {
        float[] transform;
        long timestamp;

        public RawFrame(float[] transform, long timestamp) {
            this.transform = transform;
            this.timestamp = timestamp;
        }

        @Override
        public String toString() {
            return "RawFrame{" +
                    "transform=" + Arrays.toString(transform) +
                    ", timestamp=" + timestamp +
                    '}';
        }
    }

    private static class PrepareParams {
        EGLContext sharedContext;
        int width;
        int height;
        int textureId;
        int rotation;
//        GPUImageFilter filter;

        public PrepareParams(EGLContext sharedContext, int textureId, int rotation, int width, int height) {
            this.sharedContext = sharedContext;
            this.width = width;
            this.height = height;
            this.textureId = textureId;
            this.rotation = rotation;
        }
    }
}