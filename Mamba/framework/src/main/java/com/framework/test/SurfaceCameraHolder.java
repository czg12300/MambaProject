package com.framework.test;

import android.hardware.Camera;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

/**
 * camera 和surfaceView一起使用的时候
 *
 * @author jake
 * @since 2017/2/9 下午5:32
 */

public class SurfaceCameraHolder extends BaseCameraHolder implements SurfaceHolder.Callback {
    private SurfaceHolder mSurfaceHolder;

    public SurfaceCameraHolder(SurfaceView surfaceView) {
        if (surfaceView != null) {
            mSurfaceHolder=surfaceView.getHolder();
            mSurfaceHolder.addCallback(this);
        }
    }

    public SurfaceHolder getSurfaceHolder() {
        return mSurfaceHolder;
    }

    @Override
    protected void setCameraPreviewDisplay(Camera camera) {
        try {
            camera.setPreviewDisplay(mSurfaceHolder);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        L.d("surfaceCreated");

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        L.d("surfaceChanged");
        stopPreviewAndRelease();
        openCamera(CAMERA_ID_BACK);
        startPreview();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        L.d("surfaceDestroyed");
        stopPreviewAndRelease();
    }
}
