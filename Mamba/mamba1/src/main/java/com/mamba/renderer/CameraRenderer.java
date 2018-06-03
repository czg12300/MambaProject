package com.mamba.renderer;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.EGLContext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.WindowManager;

import com.mamba.camera.CameraInstance;
import com.mamba.renderer.gpuimage.OpenGlUtils;
import com.mamba.renderer.gpuimage.Rotation;
import com.mamba.renderer.gpuimage.TextureRotationUtil;
import com.mamba.renderer.gpuimage.filter.CameraInputFilter;
import com.mamba.renderer.gpuimage.filter.GPUImageFilter;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class CameraRenderer implements GLSurfaceView.Renderer {
    private GLSurfaceView mGLSurfaceView;
    private GPUImageFilter mFilter;
    private int mTextureId = OpenGlUtils.NO_TEXTURE;
    private FloatBuffer mGLCubeBuffer;
    private FloatBuffer mGLTextureBuffer;
    private int mSurfaceWidth, mSurfaceHeight;
    private int mImageWidth, mImageHeight;
    private CameraInputFilter mCameraInputFilter;
    private SurfaceTexture mSurfaceTexture;
    private boolean mHasSurfaceCreated = false;
    private Listener mListener;
    private OnSurfaceRendererListener mSurfaceRenderer;


    public CameraRenderer() {
        mGLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mGLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
        mGLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mGLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        mCameraInputFilter = new CameraInputFilter();

    }

    public void setGLSurfaceView(GLSurfaceView surfaceView) {
        mGLSurfaceView = surfaceView;
        if (mGLSurfaceView != null) {
            mGLSurfaceView.setEGLContextClientVersion(2);
            mGLSurfaceView.setRenderer(this);
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mHasSurfaceCreated = true;
        GLES20.glDisable(GL10.GL_DITHER);
        GLES20.glClearColor(0, 0, 0, 0);
        GLES20.glEnable(GL10.GL_CULL_FACE);
        GLES20.glEnable(GL10.GL_DEPTH_TEST);
        mCameraInputFilter.init();
        if (mSurfaceTexture == null) {
            mTextureId = OpenGlUtils.getExternalOESTextureID();
            mSurfaceTexture = new SurfaceTexture(mTextureId);
            mSurfaceTexture.setOnFrameAvailableListener(onFrameAvailableListener);
        }
        if (mListener != null) {
            mListener.onSurfaceCreated(mSurfaceTexture);
        }

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        mGLSurfaceView.postDelayed(new Runnable() {
            @Override
            public void run() {
                //此时需要等open gl开始接收到数据后才开始渲染
                mGLSurfaceView.queueEvent(new Runnable() {
                    @Override
                    public void run() {
                        onFilterChanged();
                    }
                });

            }
        }, 100);
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraInputFilter.setTextureTransformMatrix(mtx);
        int id = mTextureId;
        if (mFilter == null) {
            mCameraInputFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        } else {
            id = mCameraInputFilter.onDrawToTexture(mTextureId);
            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        }
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        printFps();
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraInputFilter.setTextureTransformMatrix(mtx);
        int id = mTextureId;
        if (mFilter == null) {
            mCameraInputFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        } else {
            id = mCameraInputFilter.onDrawToTexture(id);
            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        }
        if (mSurfaceRenderer != null) {
            mSurfaceRenderer.onRenderer(EGL14.eglGetCurrentContext(), mTextureId, getRotation(), mtx, mSurfaceTexture.getTimestamp(), mImageWidth, mImageHeight);
        }
    }


    long lastTimes = 0;
    int count = 0;

    /**
     * 打印fps
     */
    private void printFps() {
        long now = System.currentTimeMillis();
        if (lastTimes > 0) {
            if (now - lastTimes >= 1000) {
                Log.d("tag", "CameraRenderer fps:" + count + " width=" + mImageWidth + " height=" + mImageHeight);
                lastTimes = now;
                count = 0;
            } else {
                count++;
            }
        } else {
            lastTimes = now;
        }
    }

    private SurfaceTexture.OnFrameAvailableListener onFrameAvailableListener = new SurfaceTexture.OnFrameAvailableListener() {

        @Override
        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
            requestRender();
        }
    };

    private void onFilterChanged() {
        mCameraInputFilter.onDisplaySizeChanged(mSurfaceWidth, mSurfaceHeight);
        if (mFilter != null) {
            if (!mFilter.isInitialized()) {
                mFilter.init();
            }
            mCameraInputFilter.initCameraFrameBuffer(mImageWidth, mImageHeight);
            mFilter.onDisplaySizeChanged(mSurfaceWidth, mSurfaceHeight);
            mFilter.onInputSizeChanged(mImageWidth, mImageHeight);
        } else {
            mCameraInputFilter.destroyFramebuffers();
        }

    }

    /**
     * 设置滤镜
     *
     * @param gpuImageFilter
     */
    public void setFilter(final GPUImageFilter gpuImageFilter) {
        if (gpuImageFilter == null) {
            return;
        }
        if (mHasSurfaceCreated) {
            mGLSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
                    if (mFilter != null) {
                        mFilter.destroy();
                    }
                    mFilter = gpuImageFilter;
                    mFilter.init();
                    onFilterChanged();
                }
            });
            requestRender();
        } else {
            mFilter = gpuImageFilter;
        }
    }


    public void requestRender() {
        if (mGLSurfaceView != null) {
            mGLSurfaceView.requestRender();
        }
    }

    private CameraInstance.CameraCallback mCameraCallback = new CameraInstance.CameraCallback() {
        @Override
        public void onCameraOpened(CameraInstance cameraInstance, int width, int height) {
            mImageWidth = width;
            mImageHeight = height;
            mCameraInputFilter.onInputSizeChanged(mImageWidth, mImageHeight);
            float[] textureCords = TextureRotationUtil.getRotation(Rotation.fromInt(getRotation()), true, false);
            mGLTextureBuffer.clear();
            mGLTextureBuffer.put(textureCords).position(0);
            cameraInstance.startPreview();
        }

        @Override
        public void onCameraClosed() {

        }
    };

    private int getRotation() {
        int result = 0;
        Display d = ((WindowManager) mGLSurfaceView.getContext().getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        switch (d.getRotation()) {
            case Surface.ROTATION_0:
                result = 180;
                break;
            case Surface.ROTATION_180:
                result = 0;
                break;
            case Surface.ROTATION_90:
                result = 90;
                break;
            case Surface.ROTATION_270:
                result = 270;
                break;
        }
        return result;
    }

    public void setOnSurfaceRendererListener(OnSurfaceRendererListener renderer) {
        this.mSurfaceRenderer = renderer;
    }

    public interface OnSurfaceRendererListener {
        void onRenderer(EGLContext sharedContext, int textureId, int rotation, float[] transform, long timestamp, int width, int height);
    }

    public CameraInstance.CameraCallback getCameraCallback() {
        return mCameraCallback;
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    public interface Listener {
        void onSurfaceCreated(SurfaceTexture texture);

    }
}