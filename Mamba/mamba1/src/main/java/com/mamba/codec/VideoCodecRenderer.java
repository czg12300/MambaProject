package com.mamba.codec;

import android.opengl.EGLContext;
import android.view.Surface;

import com.mamba.renderer.gles.EglCore;
import com.mamba.renderer.gles.WindowSurface;
import com.mamba.renderer.gpuimage.Rotation;
import com.mamba.renderer.gpuimage.TextureRotationUtil;
import com.mamba.renderer.gpuimage.filter.CameraInputFilter;
import com.mamba.renderer.gpuimage.filter.GPUImageFilter;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class VideoCodecRenderer {
    private WindowSurface mInputWindowSurface;
    private EglCore mEglCore;
    private CameraInputFilter mInput;
    private GPUImageFilter mFilter;
    private FloatBuffer gLCubeBuffer;
    private FloatBuffer gLTextureBuffer;
    private int mInputWidth;
    private int mInputHeight;
    private int mOutputWidth;
    private int mOutputHeight;
    private int mTextureId;
    private boolean mHasPrepared = false;

    public VideoCodecRenderer() {

    }


    public void release() {
        mHasPrepared = false;
        if (mInputWindowSurface != null) {
            mInputWindowSurface.release();
            mInputWindowSurface = null;
        }
        if (mEglCore != null) {
            mEglCore.release();
            mEglCore = null;
        }
        if (mInput != null) {
            mInput.destroy();
            mInput = null;
        }
        if (gLCubeBuffer != null) {
            gLCubeBuffer.clear();
        }
        if (gLTextureBuffer != null) {
            gLTextureBuffer.clear();
        }
    }

    public void prepare(EGLContext sharedContext, Surface surface, int rotation, int textureId, int inputWidth, int inputHeight, int outputWidth, int outputHeight) {
        mTextureId = textureId;
        mInputWidth = inputWidth;
        mInputHeight = inputHeight;
        mOutputWidth = outputWidth;
        mOutputHeight = outputHeight;
        gLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
        gLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        mEglCore = new EglCore(sharedContext, EglCore.FLAG_RECORDABLE);
        mInputWindowSurface = new WindowSurface(mEglCore, surface, true);
        mInputWindowSurface.makeCurrent();
        mInput = new CameraInputFilter();
        mInput.init();
        mInput.onInputSizeChanged(mInputWidth, mInputHeight);
        mInput.onDisplaySizeChanged(mOutputWidth, mOutputHeight);
        onFilterChanged();
        gLTextureBuffer.clear();
        gLTextureBuffer.put(TextureRotationUtil.getRotation(Rotation.fromInt(rotation), true, false)).position(0);
        mHasPrepared = true;
    }

    public void setFilter(GPUImageFilter filter) {
        mFilter = filter;
        if (mHasPrepared) {
            onFilterChanged();
        }
    }

    private void onFilterChanged() {
        if (mFilter != null) {
            if (!mFilter.isInitialized()) {
                mFilter.init();
            }
            mFilter.onInputSizeChanged(mInputWidth, mInputHeight);
            mFilter.onDisplaySizeChanged(mInputWidth, mInputHeight);
        }
    }

    public void drawFrame(float[] transform, long timestamp) {
        mInputWindowSurface.setPresentationTime(timestamp);
        mInput.setTextureTransformMatrix(transform);
        if (mFilter == null) {
            mInput.onDrawFrame(mTextureId, gLCubeBuffer, gLTextureBuffer);
        } else {
            int id = mInput.onDrawToTexture(mTextureId);
            mFilter.onDrawFrame(id, gLCubeBuffer, gLTextureBuffer);
        }
        mInputWindowSurface.swapBuffers();
    }

}