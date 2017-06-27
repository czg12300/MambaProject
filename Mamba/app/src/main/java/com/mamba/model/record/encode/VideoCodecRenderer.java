package com.mamba.model.record.encode;

import android.graphics.SurfaceTexture;
import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.Surface;

import com.framework.test.L;
import com.mamba.AppApplication;
import com.mamba.model.VLog;
import com.mamba.model.record.encode.gles.EglCore;
import com.mamba.model.record.encode.gles.WindowSurface;
import com.mamba.model.record.randerer.ISurfaceRenderer;
import com.mamba.model.record.randerer.gpuimage.FilterFactory;
import com.mamba.model.record.randerer.gpuimage.FilterType;
import com.mamba.model.record.randerer.gpuimage.Rotation;
import com.mamba.model.record.randerer.gpuimage.TextureRotationUtil;
import com.mamba.model.record.randerer.gpuimage.filter.CameraInputFilter;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;

import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * 渲染mediacodec的surface
 *
 * @author jake
 * @since 2017/6/22 下午6:05
 */

public class VideoCodecRenderer extends Thread implements ISurfaceRenderer {
    private WindowSurface mInputWindowSurface;
    private EglCore mEglCore;
    private CameraInputFilter mInput;
    private GPUImageFilter mFilter;
    private FloatBuffer gLCubeBuffer;
    private FloatBuffer gLTextureBuffer;
    private OnRendererListener onRendererListener;
    private Surface mSurface;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private volatile boolean mIsRenderer = false;
    private volatile boolean mIsPrepare = false;
    private PrepareParams mPrepareParams;

    public void setOnRendererListener(OnRendererListener onRendererListener) {
        this.onRendererListener = onRendererListener;
    }

    public VideoCodecRenderer(Surface surface, int width, int height) {
        mSurface = surface;
        mVideoWidth = width;
        mVideoHeight = height;
        gLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
        gLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        mRawFrameList = new LinkedBlockingQueue<>();
    }

    @Override
    public synchronized void start() {
        super.start();
//        mRawFrameList.clear();
        mIsRenderer = true;
    }

    public synchronized void stopEncode() {
        mIsRenderer = false;
        mIsPrepare = false;
    }

    private void release() {
        if (mInputWindowSurface != null) {
            mInputWindowSurface.release();
            mInputWindowSurface = null;
        }
        if (mInput != null) {
            mInput.destroy();
            mInput = null;
        }
        if (mEglCore != null) {
            mEglCore.release();
            mEglCore = null;
        }
        if (mFilter != null) {
            mFilter.destroy();
            mFilter = null;
        }
    }

    private void prepare(EGLContext sharedContext, int width, int height, GPUImageFilter filter) {
        VLog.d("prepare");
        mPreviewWidth = width;
        mPreviewHeight = height;
        mEglCore = new EglCore(sharedContext, EglCore.FLAG_RECORDABLE);
        VLog.d("mSurface is null" + (mSurface == null));
        mInputWindowSurface = new WindowSurface(mEglCore, mSurface, true);
        mInputWindowSurface.makeCurrent();
        VLog.d("WindowSurface success");
        mInput = new CameraInputFilter();
        mInput.init();
        mFilter = filter;
        if (mFilter != null) {
            mFilter.init();
            mFilter.onInputSizeChanged(mPreviewWidth, mPreviewHeight);
            mFilter.onDisplaySizeChanged(mVideoWidth, mVideoHeight);
        }
        gLTextureBuffer.clear();
        gLTextureBuffer.put(TextureRotationUtil.getRotation(Rotation.fromInt(90),
                false, true)).position(0);
        VLog.d("prepare success");
    }

    @Override
    public void run() {
        super.run();
        while (true) {
            if (mPrepareParams != null) {
                prepare(mPrepareParams.sharedContext, mPrepareParams.width, mPrepareParams.height, mPrepareParams.filter);
                if (onRendererListener != null) {
                    onRendererListener.onStart();
                }
                mPrepareParams = null;
                mIsPrepare = true;
                break;
            } else {
                try {
                    sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        while (true) {
            int size = 0;
            synchronized (mRawFrameList) {
                size = mRawFrameList.size();
            }
            VLog.d("mRawFrameList  size " + size);
            if (size > 0) {
                RawFrame frame = null;
                synchronized (mRawFrameList) {
                    frame = mRawFrameList.poll();
                }
                handFrameUpdate(frame);

            } else {
                try {
                    sleep(30);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!mIsRenderer) {
                break;
            }
        }
        if (!mRawFrameList.isEmpty()) {//flush raw frame
            for (RawFrame frame : mRawFrameList) {
                handFrameUpdate(frame);
            }
        }
        if (onRendererListener != null) {
            onRendererListener.onStop();
        }
        release();
    }


    private void handFrameUpdate(RawFrame rawFrame) {
        if (rawFrame == null) {
            return;
        }
        VLog.d("handFrameUpdate start");
        if (onRendererListener != null) {
            int len = onRendererListener.getRendererFrameTimes(rawFrame.timestamp);
            VLog.d("handFrameUpdate getRendererFrameTimes " + len);
            if (len > 0) {
                for (int i = 0; i < len; i++) {
                    onRendererListener.onRenderer();
                    mInput.setTextureTransformMatrix(rawFrame.transform);
                    if (mFilter == null) {
                        mInput.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
                    } else {
                        mFilter.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
                    }
//                    mInputWindowSurface.swapBuffers();
                }
            }

        }

    }

    @Override
    public void onRenderer(EGLContext sharedContext, int textureId, float[] transform, long timestamp, int width, int height, FilterType type) {
        if (!mIsRenderer) {
            return;
        }
        VLog.e("onRenderer    (EGLContext sharedContext");
        if (!mIsPrepare) {
            VLog.d("prepare");
            mPrepareParams = new PrepareParams(sharedContext, width, height, FilterFactory.getFilter(AppApplication.get().getApplicationContext(), type));
        }
        if (timestamp == 0) {
            // Seeing this after device is toggled off/on with power button.  The
            // first frame back has a zero timestamp.
            // MPEG4Writer thinks this is cause to abort() in native code, so it's very
            // important that we just ignore the frame.
            return;
        }
        synchronized (mRawFrameList) {
            VLog.d("onRenderer  mRawFrameList  size " + mRawFrameList.size());
//            if(onRendererListener!=null){
//             int times=   onRendererListener.getRendererFrameTimes(timestamp);
//                for (int i<0;)
            mRawFrameList.offer(new RawFrame(textureId, transform, timestamp));
//            }
        }
        VLog.d("offer success");
    }

    private Queue<RawFrame> mRawFrameList;

    public static interface OnRendererListener {
        int getRendererFrameTimes(long timestamp);

        int getPositionFrameRate();

        void onStart();

        void onRenderer();

        void onStop();
    }

    private static class RawFrame {
        float[] transform;
        long timestamp;
        int textureId;

        public RawFrame(int textureId, float[] transform, long timestamp) {
            this.textureId = textureId;
            this.transform = transform;
            this.timestamp = timestamp;
        }
    }

    private static class PrepareParams {
        EGLContext sharedContext;
        int width;
        int height;
        GPUImageFilter filter;

        public PrepareParams(EGLContext sharedContext, int width, int height, GPUImageFilter filter) {
            this.sharedContext = sharedContext;
            this.width = width;
            this.height = height;
            this.filter = filter;
        }
    }
}
