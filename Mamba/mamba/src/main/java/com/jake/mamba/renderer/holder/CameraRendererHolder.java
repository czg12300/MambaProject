package com.jake.mamba.renderer.holder;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;

import com.jake.mamba.camera.CameraInstance;
import com.jake.mamba.camera.RecordCamera;
import com.jake.mamba.renderer.BaseRenderer;
import com.jake.mamba.renderer.CameraRenderer;
import com.jake.mamba.utils.L;

/**
 * @author jake
 * @since 2018/5/4 下午6:12
 */

public class CameraRendererHolder implements IRendererHolder, BaseRenderer.RendererCallback {
    private GLSurfaceView mGLSurfaceView;
    private CameraRenderer mCameraRenderer;

    public CameraRendererHolder(GLSurfaceView surfaceView) {
        mGLSurfaceView = surfaceView;
        mCameraRenderer = new CameraRenderer();
        mCameraRenderer.setRendererCallback(this);
        mGLSurfaceView.setEGLContextClientVersion(2);
        mGLSurfaceView.setRenderer(mCameraRenderer);
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        CameraInstance.get().checkAndInit(surfaceView.getContext());
        CameraInstance.get().registerCameraCallback(mCameraRenderer
        );
    }

    @Override
    public void onResume() {
//        mGLSurfaceView.onResume();
    }

    @Override
    public void onPause() {
//        mGLSurfaceView.onPause();
    }

    @Override
    public void onDestroy() {
    }

    @Override
    public void onCreated() {
        mCameraRenderer.geSurfaceTexture().setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
            @Override
            public void onFrameAvailable(SurfaceTexture surfaceTexture) {
                mGLSurfaceView.requestRender();
            }
        });
        CameraInstance.get().setDisplay(mCameraRenderer.geSurfaceTexture());
        mGLSurfaceView.post(new Runnable() {
            @Override
            public void run() {
                CameraInstance.get().openFrontCamera();
            }
        });
    }

    @Override
    public void onChanged(int width, int height) {

    }

    @Override
    public void onDraw() {

    }
}
