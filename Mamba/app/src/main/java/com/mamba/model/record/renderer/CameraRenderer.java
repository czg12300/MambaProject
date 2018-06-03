package com.mamba.model.record.renderer;

import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import com.mamba.model.VLog;
import com.mamba.model.record.RecordCamera;
import com.mamba.model.record.renderer.gpuimage.FilterFactory;
import com.mamba.model.record.renderer.gpuimage.OpenGlUtils;
import com.mamba.model.record.renderer.gpuimage.Rotation;
import com.mamba.model.record.renderer.gpuimage.TextureRotationUtil;
import com.mamba.model.record.renderer.gpuimage.filter.CameraInputFilter;
import com.mamba.model.record.renderer.gpuimage.filter.GPUImageFilter;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * camera 渲染器，主要实现GLSurfaceView.Renderer，渲染摄像头的数据，并实现实时滤镜
 *
 * @author jake
 * @since 2017/3/29 下午2:26
 */
public class CameraRenderer implements GLSurfaceView.Renderer, RecordCamera.Callback {
    public GLSurfaceView mGLSurfaceView;
//    protected GPUImageFilter mFilter;
//    protected int mTextureId = OpenGlUtils.NO_TEXTURE;
//    protected final FloatBuffer mGLCubeBuffer;
//    protected final FloatBuffer mGLTextureBuffer;
    protected int mSurfaceWidth, mSurfaceHeight;
    private final CameraInputFilter mCameraInputFilter;
    private SurfaceTexture mSurfaceTexture;
    private boolean mIsSurfaceCreated = false;

    private RecordCamera mCamera;
    private int mPreviewWidth = 720;
    private int mPreviewHeight = 1280;
    private int mPreviewFormat = ImageFormat.YV12;

    public boolean isSurfaceCreated() {
        return mIsSurfaceCreated;
    }

    private ISurfaceRenderer mSurfaceRenderer;

    public CameraRenderer() {
//        mGLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4)
//                .order(ByteOrder.nativeOrder())
//                .asFloatBuffer();
//        mGLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
//        mGLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4)
//                .order(ByteOrder.nativeOrder())
//                .asFloatBuffer();
//        mGLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        mCameraInputFilter = new CameraInputFilter();

    }

    public void setGLSurfaceView(GLSurfaceView surfaceView) {
        mGLSurfaceView = surfaceView;
        if (mGLSurfaceView != null) {
            mCamera = new RecordCamera(mGLSurfaceView.getContext());
            mCamera.addCallback(this);
            mCamera.setParameters(mPreviewFormat, mPreviewWidth, mPreviewHeight);
            mGLSurfaceView.setEGLContextClientVersion(2);
            mGLSurfaceView.setRenderer(this);
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }

    public void setPreviewFormat(int previewFormat) {
        mPreviewFormat = previewFormat;
    }

    public void setPreviewSize(int width, int height) {
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    public void setSurfaceRenderer(ISurfaceRenderer mSurfaceRenderer) {
        this.mSurfaceRenderer = mSurfaceRenderer;
    }

    public RecordCamera getCamera() {
        return mCamera;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
//        mIsSurfaceCreated = true;
//        GLES20.glDisable(GL10.GL_DITHER);
//        GLES20.glClearColor(0, 0, 0, 0);
//        GLES20.glEnable(GL10.GL_CULL_FACE);
//        GLES20.glEnable(GL10.GL_DEPTH_TEST);
        mCameraInputFilter.init();
        mSurfaceTexture = new SurfaceTexture(mCameraInputFilter.getTextureId());
        mSurfaceTexture.setOnFrameAvailableListener(onFrameAvailableListener);
        if (mCamera != null) {
            mCamera.setDisplay(mSurfaceTexture);
            mGLSurfaceView.post(new Runnable() {
                @Override
                public void run() {
                    mCamera.openFrontCamera();
                }
            });

        }
//        if (mTextureId == OpenGlUtils.NO_TEXTURE) {
//            mTextureId = OpenGlUtils.getExternalOESTextureID();
//            if (mTextureId != OpenGlUtils.NO_TEXTURE) {
//                mSurfaceTexture = new SurfaceTexture(mTextureId);
//                mSurfaceTexture.setOnFrameAvailableListener(onFrameAvailableListener);
//                if (mCamera != null) {
//                    mCamera.setDisplay(mSurfaceTexture);
//                    mGLSurfaceView.post(new Runnable() {
//                        @Override
//                        public void run() {
//                            mCamera.openFrontCamera();
//                        }
//                    });
//
//                }
//
//            }
//        }

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
//        GLES20.glViewport(0, 0, width, height);
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        mCameraInputFilter.onDisplaySizeChanged(mSurfaceWidth, mSurfaceHeight);
//        mCameraInputFilter.destroyFramebuffers();
//        mSurfaceTexture.updateTexImage();
//        float[] mtx = new float[16];
//        mSurfaceTexture.getTransformMatrix(mtx);
//        mCameraInputFilter.setTextureTransformMatrix(mtx);
//        mCameraInputFilter.onDrawFrame(mTextureId, mGLCubeBuffer, mGLTextureBuffer);



//        onFilterChanged();
//        if (mSurfaceTexture == null)
//            return;
//        mSurfaceTexture.updateTexImage();
//        float[] mtx = new float[16];
//        mSurfaceTexture.getTransformMatrix(mtx);
//        mCameraInputFilter.setTextureTransformMatrix(mtx);
//        mCameraInputFilter.onDrawFrame(mTextureId, mGLCubeBuffer, mGLTextureBuffer);
//        int id = mTextureId;
//        if (mFilter == null) {
//            mCameraInputFilter.onDrawFrame(mTextureId, mGLCubeBuffer, mGLTextureBuffer);
//        } else {
//            id = mCameraInputFilter.onDrawToTexture(mTextureId);
//            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
//        }
    }


    @Override
    public void onDrawFrame(GL10 gl) {
        printFps();
        GLES20.glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        if (mSurfaceTexture == null) {
            return;
        }
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraInputFilter.setTextureTransformMatrix(mtx);
//        if (mFilter == null) {
            mCameraInputFilter.onDrawFrame( );
//        } else {
//            id = mCameraInputFilter.onDrawToTexture(mTextureId);
//            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
//        }
//        if (mSurfaceRenderer != null) {
//            long timestamp = mSurfaceTexture.getTimestamp();
//            mSurfaceRenderer.onRenderer(EGL14.eglGetCurrentContext(), id, mtx, timestamp, mPreviewWidth, mPreviewHeight, FilterFactory.getFilterType(mFilter));
//        }
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
                VLog.d("CameraRenderer fps:" + count);
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
//        if (mFilter != null) {
//            mCameraInputFilter.initCameraFrameBuffer(mPreviewWidth, mPreviewHeight);
//            mFilter.onDisplaySizeChanged(mSurfaceWidth, mSurfaceHeight);
//            mFilter.onInputSizeChanged(mPreviewWidth, mPreviewHeight);
//        } else {
            mCameraInputFilter.destroyFramebuffers();
//        }

    }

    /**
     * 设置滤镜
     *
     * @param gpuImageFilter
     */
    public void setFilter(final GPUImageFilter gpuImageFilter) {
        if (mIsSurfaceCreated) {
            mGLSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
//                    if (mFilter != null) {
//                        mFilter.destroy();
//                    }
//                    mFilter = null;
//                    mFilter = gpuImageFilter;
//                    if (mFilter != null) {
//                        mFilter.init();
//                    }
                    onFilterChanged();
                }
            });
            requestRender();
        } else {
            mGLSurfaceView.post(new Runnable() {
                @Override
                public void run() {
                    setFilter(gpuImageFilter);
                }
            });
        }
    }

    public GPUImageFilter getFilter() {
//        return mFilter;
        return null;
    }

    public void requestRender() {
        if (mGLSurfaceView != null) {
            mGLSurfaceView.requestRender();
        }
    }


    @Override
    public void onOpened(RecordCamera camera, int width, int height) {
        camera.startPreview();
        mPreviewWidth = width;
        mPreviewHeight = height;
        mCameraInputFilter.onInputSizeChanged(mPreviewWidth, mPreviewHeight);
//        float[] textureCords = TextureRotationUtil.getRotation(Rotation.fromInt(camera.isFacingFront() ? 180 : 180),
//                false, false);
//        mGLTextureBuffer.clear();
//        mGLTextureBuffer.put(textureCords).position(0);
    }

    @Override
    public void onClosed() {

    }
}
