package com.jake.mamba.renderer;

import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLU;
import android.util.Log;

import com.jake.mamba.camera.CameraInstance;
import com.jake.mamba.opengl.EglUtils;
import com.jake.mamba.renderer.filter.CameraFilter;
import com.jake.mamba.renderer.filter.CameraInputFilter;
import com.jake.mamba.renderer.filter.FboFilter;
import com.jake.mamba.renderer.filter.LookupFilter;
import com.jake.mamba.utils.L;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class CameraRenderer extends BaseRenderer implements CameraInstance.CameraCallback {
    private CameraFilter mCameraFilter;
    private int mCameraWidth;
    private int mCameraHeight;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private SurfaceTexture mSurfaceTexture;
    public CameraRenderer() {
        mCameraFilter = new CameraFilter();
    }

    @Override
    protected void onDraw(GL10 gl) {
        printFps();
//        GLES20.glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
//        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        mSurfaceTexture.updateTexImage();
        float[] mtx = new float[16];
        mSurfaceTexture.getTransformMatrix(mtx);
        mCameraFilter.setTexMtx(mtx);
        mCameraFilter.draw();
        L.d("onDraw  mSurfaceWidth " + mSurfaceWidth + " mSurfaceHeight  " + mSurfaceHeight+"  mCameraWidth  "+mCameraWidth+"   mCameraHeight "+mCameraHeight);
    }

    @Override
    public void onCreated(GL10 gl, EGLConfig config) {
        mCameraFilter.prepare();
        if (mSurfaceTexture == null) {
            mSurfaceTexture = new SurfaceTexture(mCameraFilter.getTextureId());
        }
    }

    public SurfaceTexture geSurfaceTexture() {
        return mSurfaceTexture;
    }


    @Override
    public void onChanged(GL10 gl, int width, int height) {

        mSurfaceWidth = width;
        mSurfaceHeight = height;
        mCameraFilter.setViewport(width, height);
    }


    @Override
    public void onOpened(int width, int height) {
        L.d("onOpened  width " + width + " height  " + height);
        mCameraHeight = height;
        mCameraWidth = width;
        mCameraFilter.setOutputSize(mCameraWidth, mCameraHeight);
    }

    @Override
    public void onClosed() {
        L.d("onClosed   " );
    }

    @Override
    public void onError(int code, String msg) {
        L.d("onError  code " + code + " msg  " + msg);
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
                L.d("CameraRenderer fps:" + count);
                lastTimes = now;
                count = 0;
            } else {
                count++;
            }
        } else {
            lastTimes = now;
        }
    }


}