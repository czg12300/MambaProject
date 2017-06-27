package com.mamba.model.record.encode;

import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.Surface;

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
public class VideoCodecRendererOld implements ISurfaceRenderer, Handler.Callback {
    private WindowSurface mInputWindowSurface;
    private EglCore mEglCore;
    private CameraInputFilter mInput;
    private GPUImageFilter mFilter;
    private FloatBuffer gLCubeBuffer;
    private FloatBuffer gLTextureBuffer;
    private Handler mThreadHandler;
    private OnRendererListener onRendererListener;
    private Surface mSurface;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private volatile boolean mIsRenderer = false;
    private volatile boolean mIsPrepare = false;

    public void setOnRendererListener(OnRendererListener onRendererListener) {
        this.onRendererListener = onRendererListener;
    }

    public VideoCodecRendererOld(Surface surface, int width, int height) {
        mSurface = surface;
        mVideoWidth = width;
        mVideoHeight = height;
        gLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
        gLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);

    }

    public void start() {
        HandlerThread thread = new HandlerThread("VideoCodecRenderer");
        thread.start();
        mThreadHandler = new Handler(thread.getLooper(), this);
        mIsRenderer = true;
    }

    public void stop() {
        mIsRenderer = false;
        mIsPrepare = false;
        mThreadHandler.sendEmptyMessage(MSG_THREAD_STOP);
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
        if (mThreadHandler != null) {
            mThreadHandler.removeMessages(MSG_THREAD_UPDATE);
            mThreadHandler.getLooper().quit();
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
        gLTextureBuffer.put(TextureRotationUtil.getRotation(Rotation.fromInt(270),
                false, true)).position(0);
        VLog.d("prepare success");
    }


    private static final int MSG_THREAD_START = 0x01;
    private static final int MSG_THREAD_UPDATE = 0x02;
    private static final int MSG_THREAD_STOP = 0x03;

    @Override
    public boolean handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_THREAD_START:
                if (msg.obj != null) {
                    PrepareParams pp = (PrepareParams) msg.obj;
                    prepare(pp.sharedContext, pp.width, pp.height, pp.filter);
                }
                break;
            case MSG_THREAD_UPDATE:
                VLog.d("handleMessage start");
                if (msg.obj != null) {
                    handFrameUpdate((RawFrame) msg.obj);
                }
                VLog.d("handleMessage success");
                break;
            case MSG_THREAD_STOP:
                if (onRendererListener != null) {
                    onRendererListener.onStop();
                }
                release();
                break;
        }
        return true;
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
                    VLog.d("handFrameUpdate  for (int i = 0; i < len; i++)   i " + i + "start");
                    onRendererListener.onRenderer();
                    mInput.setTextureTransformMatrix(rawFrame.transform);
                    if (mFilter == null) {
                        mInput.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
                    } else {
                        mFilter.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
                    }
                    mInputWindowSurface.setPresentationTime(rawFrame.timestamp);
                    mInputWindowSurface.swapBuffers();
                    VLog.d("handFrameUpdate  for (int i = 0; i < len; i++)   i " + i + "end");
                }
            }

        }

    }

    @Override
    public void onRenderer(EGLContext sharedContext, int textureId, float[] transform, long timestamp, int width, int height, FilterType filter) {
        VLog.d("onRenderer FPS");
        if (!mIsRenderer) {
            return;
        }
        if (!mIsPrepare) {
            VLog.d("prepare");
            mThreadHandler.sendMessage(mThreadHandler.obtainMessage(MSG_THREAD_START, new PrepareParams(sharedContext, width, height, FilterFactory.getFilter(AppApplication.get().getApplicationContext(), filter))));
            mIsPrepare = true;

        }
        if (timestamp == 0) {
            // Seeing this after device is toggled off/on with power button.  The
            // first frame back has a zero timestamp.
            // MPEG4Writer thinks this is cause to abort() in native code, so it's very
            // important that we just ignore the frame.
            return;
        }
        mThreadHandler.sendMessage(mThreadHandler.obtainMessage(MSG_THREAD_UPDATE, new RawFrame(textureId, transform, timestamp)));
        VLog.d("sendMessage success");
    }

    public static interface OnRendererListener {
        int getRendererFrameTimes(long timestamp);

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

