package com.mamba.model.record.randerer;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;


import com.mamba.model.VLog;
import com.mamba.model.record.camera.CameraImp;
import com.mamba.model.record.camera.CameraImpFactory;
import com.mamba.model.record.encode.video.OnVideoFrameAvailableListener;
import com.mamba.model.record.encode.video.VideoFrame;
import com.mamba.model.record.randerer.gpuimage.OpenGlUtils;
import com.mamba.model.record.randerer.gpuimage.Rotation;
import com.mamba.model.record.randerer.gpuimage.TextureRotationUtil;
import com.mamba.model.record.randerer.gpuimage.filter.CameraInputFilter;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;

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
public class CameraRenderer implements GLSurfaceView.Renderer, CameraImp.CameraImpCallback, CameraImp.PreviewCallback {
    public GLSurfaceView mGLSurfaceView;
    protected GPUImageFilter mFilter;
    protected int mTextureId = OpenGlUtils.NO_TEXTURE;
    protected final FloatBuffer mGLCubeBuffer;
    protected final FloatBuffer mGLTextureBuffer;
    protected int mSurfaceWidth, mSurfaceHeight;
    protected int mImageWidth, mImageHeight;
    private final CameraInputFilter mCameraInputFilter;
    private SurfaceTexture mSurfaceTexture;
    private boolean mIsSurfaceCreated = false;
    private OnVideoFrameAvailableListener frameAvailableListener;

    public boolean isSurfaceCreated() {
        return mIsSurfaceCreated;
    }

    private CameraImp mCameraImp;


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
            mCameraImp = CameraImpFactory.getCameraImp(mGLSurfaceView.getContext().getApplicationContext());
            mCameraImp.addCameraImpCallback(this);
            mCameraImp.addPreviewCallback(this);
            mGLSurfaceView.setEGLContextClientVersion(2);
            mGLSurfaceView.setRenderer(this);
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
    }


    public CameraImp getCameraImp() {
        return mCameraImp;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mIsSurfaceCreated = true;
        GLES20.glDisable(GL10.GL_DITHER);
        GLES20.glClearColor(0, 0, 0, 0);
        GLES20.glEnable(GL10.GL_CULL_FACE);
        GLES20.glEnable(GL10.GL_DEPTH_TEST);
        mCameraInputFilter.init();
        if (mTextureId == OpenGlUtils.NO_TEXTURE) {
            mTextureId = OpenGlUtils.getExternalOESTextureID();
            if (mTextureId != OpenGlUtils.NO_TEXTURE) {
                mSurfaceTexture = new SurfaceTexture(mTextureId);
                mSurfaceTexture.setOnFrameAvailableListener(onFrameAvailableListener);
                if (mCameraImp != null) {
                    mCameraImp.setDisplay(mSurfaceTexture);
                    mCameraImp.openBackCamera();
                }

            }
        }

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        onFilterChanged();
        if (mSurfaceTexture == null)
            return;
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraInputFilter.setTextureTransformMatrix(mtx);
        int id = mTextureId;
        if (mFilter == null) {
            mCameraInputFilter.onDrawFrame(mTextureId, mGLCubeBuffer, mGLTextureBuffer);
        } else {
            id = mCameraInputFilter.onDrawToTexture(mTextureId);
            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        }

    }


    @Override
    public void onDrawFrame(GL10 gl) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        if (mSurfaceTexture == null) {
            return;
        }
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraInputFilter.setTextureTransformMatrix(mtx);
        int id = mTextureId;
        if (mFilter == null) {
            mCameraInputFilter.onDrawFrame(mTextureId, mGLCubeBuffer, mGLTextureBuffer);
        } else {
            id = mCameraInputFilter.onDrawToTexture(mTextureId);
            mFilter.onDrawFrame(id, mGLCubeBuffer, mGLTextureBuffer);
        }
//        VLog.d("onDrawFrame   timestamp  " + mSurfaceTexture.getTimestamp());
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
        if (mFilter != null) {
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
        if (mIsSurfaceCreated) {
            mGLSurfaceView.queueEvent(new Runnable() {
                @Override
                public void run() {
                    if (mFilter != null) {
                        mFilter.destroy();
                    }
                    mFilter = null;
                    mFilter = gpuImageFilter;
                    if (mFilter != null) {
                        mFilter.init();
                    }
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
        return mFilter;
    }

    public void requestRender() {
        if (mGLSurfaceView != null) {
            mGLSurfaceView.requestRender();
        }
    }


    @Override
    public void onCameraOpened(CameraImp cameraImp, int width, int height) {
        cameraImp.startPreview();
        mImageWidth = width;
        mImageHeight = height;
        mCameraInputFilter.onInputSizeChanged(mImageWidth, mImageHeight);
        float[] textureCords = TextureRotationUtil.getRotation(Rotation.fromInt(cameraImp.isFacingFront() ? 270 : 270),
                true, false);
        mGLTextureBuffer.clear();
        mGLTextureBuffer.put(textureCords).position(0);
    }

    @Override
    public void onCameraClosed() {

    }

    public void setFrameAvailableListener(OnVideoFrameAvailableListener frameAvailableListener) {
        this.frameAvailableListener = frameAvailableListener;
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height, CameraImp cameraImp) {
        printFps();
        if (frameAvailableListener != null) {
            VideoFrame frame = VideoFrame.create(width, height, data, System.currentTimeMillis(), cameraImp.isFacingFront() ? 270 : 90);
            frameAvailableListener.onFrameAvailable(frame);
        }
    }
}
