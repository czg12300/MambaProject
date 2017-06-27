package com.mamba.model.record.camera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.support.v4.app.ActivityCompat;
import android.view.SurfaceHolder;


import com.mamba.model.VLog;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * 使用老的camera api
 *
 * @author jake
 * @since 2017/4/27 上午11:43
 */

public class Camera1 implements CameraImp {
    /**
     * 前置摄像头id
     */
    public int CAMERA_ID_FRONT = 1;
    /**
     * 后置摄像头id
     */
    public int CAMERA_ID_BACK = 0;
    protected Camera mCamera;
    protected int mCurrentCameraId = 0;
    private Size mPreviewSize = new Size(720, 1280);
    private Size mPictureSize = new Size(720, 1280);
    private int pictureFormat = ImageFormat.JPEG;
    private int previewFormat = ImageFormat.NV21;
    private ArrayList<PictureCallback> mPictureCallbackList;
    private ArrayList<PreviewCallback> mPreviewCallbackList;
    private ArrayList<CameraImpCallback> mCameraImpCallbackList;
    private SurfaceTexture mPreviewSurfaceTexture;
    private SurfaceHolder mPreviewSurfaceHolder;
    private boolean hasAvailableCamera = false;
    private Context mAppContext;

    public Camera1(Context context) {
        mAppContext = context.getApplicationContext();
        init();
    }

    private void init() {
        int num = Camera.getNumberOfCameras();
        VLog.d("num=" + num);
        VLog.d("CAMERA_ID_BACK=" + CAMERA_ID_BACK);
        VLog.d("CAMERA_ID_FRONT=" + CAMERA_ID_FRONT);
        if (num > 0) {
            hasAvailableCamera = true;
            for (int i = 0; i < num; i++) {
                Camera.CameraInfo info = new Camera.CameraInfo();
                Camera.getCameraInfo(i, info);
                if (info.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    CAMERA_ID_BACK = i;
                } else if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    CAMERA_ID_FRONT = i;
                }
            }
        } else {
            hasAvailableCamera = false;
        }
        VLog.d("CAMERA_ID_BACK=" + CAMERA_ID_BACK);
        VLog.d("CAMERA_ID_FRONT=" + CAMERA_ID_FRONT);
    }


    private void openCamera() {
        if (!hasAvailableCamera) {
            return;
        }
        if (ActivityCompat.checkSelfPermission(mAppContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            VLog.e("没有照相机权限");
            return;
        }
        if (mCamera != null) {
            releaseCamera();
        }
        try {
            mCamera = Camera.open(mCurrentCameraId);
        } catch (Exception e) {
            e.printStackTrace();
            mCamera = null;
        }
        if (!isCameraOpened()) {
            return;
        }
        updateCameraParameters();
        if (mPreviewSurfaceTexture != null) {
            try {
                mCamera.setPreviewTexture(mPreviewSurfaceTexture);
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }
        }
        if (mPreviewSurfaceHolder != null) {
            try {
                mCamera.setPreviewDisplay(mPreviewSurfaceHolder);
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }
        }
        if (mCameraImpCallbackList != null) {
            for (CameraImpCallback callback : mCameraImpCallbackList) {
                if (callback != null) {
                    callback.onCameraOpened(getCameraImp(), mPreviewSize.width, mPreviewSize.height);
                }
            }
        }
    }

    /**
     * 更新摄像头参数设置
     */
    private void updateCameraParameters() {
        Camera.Parameters parameters = mCamera.getParameters();

        if (parameters.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_AUTO)){
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
        }
        mPreviewSize = CameraUtils.getLargeSize(CameraUtils.transArrayToList(parameters.getSupportedPreviewSizes()), mPreviewSize.width, mPreviewSize.height);
        mPictureSize = CameraUtils.getLargeSize(CameraUtils.transArrayToList(parameters.getSupportedPictureSizes()), mPictureSize.width, mPictureSize.height);
        parameters.setPreviewSize(mPreviewSize.width, mPreviewSize.height);
        parameters.setPictureSize(mPictureSize.width, mPictureSize.height);
        if (parameters.getSupportedFocusModes().contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
        }
        if (parameters.isZoomSupported()) {
            parameters.setZoom(0);
        }
        List<int[]> fpsList = parameters.getSupportedPreviewFpsRange();
        if (fpsList != null && fpsList.size() > 0) {
            int[] range = {0, 0};
            for (int[] num : fpsList) {
                if (num[0] > range[0]) {
                    range[0] = num[0];
                    range[1] = num[1];
                    VLog.d("fpsList " + num[0] + " : " + num[1]);
                }
            }
            parameters.setPreviewFpsRange(range[0], range[1]);
        }
        parameters.setPictureFormat(pictureFormat);
        parameters.setPreviewFormat(previewFormat);
        parameters.setRecordingHint(true);
        mCamera.setParameters(parameters);
    }


    @Override
    public void takePicture() {
        mCamera.takePicture(null, null, null, new Camera.PictureCallback() {
            @Override
            public void onPictureTaken(byte[] data, Camera camera) {
                if (mPictureCallbackList != null) {
                    for (PictureCallback callback : mPictureCallbackList) {
                        if (callback != null) {
                            callback.onPictureFrame(data, mPictureSize.width, mPictureSize.height, getCameraImp());
                        }
                    }
                }
            }
        });
    }

    private CameraImp getCameraImp() {
        return this;
    }

    @Override
    public void stopPreview() {
        if (isCameraOpened()) {
            mCamera.stopPreview();//停掉原来摄像头的预览
        }
    }

    @Override
    public void startPreview() {
        if (isCameraOpened()) {
            mCamera.addCallbackBuffer(new byte[mPreviewSize.width * mPreviewSize.height * 3 / 2]);
            mCamera.setPreviewCallbackWithBuffer(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] data, Camera camera) {
                    camera.addCallbackBuffer(data);
                    if (mPreviewCallbackList != null) {
                        for (PreviewCallback callback : mPreviewCallbackList) {
                            if (callback != null) {
                                callback.onPreviewFrame(data, mPreviewSize.width, mPreviewSize.height, getCameraImp());
                            }
                        }
                    }
                }
            });
            mCamera.startPreview();
        }
    }

    @Override
    public void toggleCamera() {
        if (mCurrentCameraId == CAMERA_ID_FRONT) {
            openBackCamera();
        } else {
            openFrontCamera();
        }
    }

    @Override
    public void openBackCamera() {
        mCurrentCameraId = CAMERA_ID_BACK;
        openCamera();
    }

    @Override
    public void openFrontCamera() {
        mCurrentCameraId = CAMERA_ID_FRONT;
        openCamera();
    }

    @Override
    public boolean isFacingFront() {
        boolean result = false;
        if (isCameraOpened()) {
            Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
            Camera.getCameraInfo(mCurrentCameraId, cameraInfo);
            result = cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT;
        }
        return result;
    }


    @Override
    public void setZoom(int zoom) {
        if (isCameraOpened()) {
            Camera.Parameters parameters = mCamera.getParameters();
            if (parameters.isZoomSupported()) {
                parameters.setZoom(zoom);
                mCamera.setParameters(parameters);
            }
        }
    }

    @Override
    public int getZoom() {
        int ret = 0;
        if (isCameraOpened()) {
            Camera.Parameters parameters = mCamera.getParameters();
            if (parameters.isZoomSupported()) {
                ret = parameters.getZoom();
            }
        }
        return ret;
    }

    @Override
    public int getMaxZoom() {
        int ret = 0;
        if (isCameraOpened()) {
            Camera.Parameters parameters = mCamera.getParameters();
            if (parameters.isZoomSupported()) {
                ret = parameters.getMaxZoom();
            }
        }
        return ret;
    }

    @Override
    public void setParameters(CameraImpParameters parameters) {
        if (parameters != null) {
            pictureFormat = parameters.pictureFormat;
            previewFormat = parameters.previewFormat;
            if (parameters.previewSize != null) {
                mPreviewSize = parameters.previewSize;
            }
            if (parameters.pictureSize != null) {
                mPictureSize = parameters.pictureSize;
            }
        }
    }

    @Override
    public void setDisplay(SurfaceTexture surfaceTexture) {
        mPreviewSurfaceTexture = surfaceTexture;
    }

    @Override
    public void setDisplay(SurfaceHolder holder) {
        mPreviewSurfaceHolder = holder;
    }


    @Override
    public void release() {
        releaseCamera();
        if (mCameraImpCallbackList!=null){
            mCameraImpCallbackList.clear();
            mCameraImpCallbackList=null;
        }
        if (mPictureCallbackList!=null){
            mPictureCallbackList.clear();
            mPictureCallbackList=null;
        }
        if (mPreviewCallbackList!=null){
            mPreviewCallbackList.clear();
            mPreviewCallbackList=null;
        }
    }

    private void releaseCamera() {
        if (mCamera != null) {
            stopPreview();
            mCamera.setPreviewCallback(null);
            mCamera.release();//释放资源
            mCamera = null;//取消原来摄像头
        }
    }

    @Override
    public boolean zoomIn() {
        if (isCameraOpened()) {
            int max = getMaxZoom();
            if (max > 0) {
                int zoom = getZoom();
                zoom += max / 10;
                if (zoom > max) {
                    zoom = max;
                }
                setZoom(zoom);
                return true;
            }

        }
        return false;
    }

    @Override
    public boolean zoomOut() {
        if (isCameraOpened()) {
            int max = getMaxZoom();
            if (max > 0) {
                int zoom = getZoom();
                zoom -= max / 10;
                if (zoom < 0) {
                    zoom = 0;
                }
                setZoom(zoom);
                return true;
            }

        }
        return false;
    }

    @Override
    public void openFlashLight() {
        if (isCameraOpened()) {
            Camera.Parameters parameter = mCamera.getParameters();
            parameter.setFlashMode(Camera.Parameters.FLASH_MODE_TORCH);
            mCamera.setParameters(parameter);
        }
    }

    @Override
    public void closeFlashLight() {
        if (isCameraOpened()) {
            Camera.Parameters parameter = mCamera.getParameters();
            parameter.setFlashMode(Camera.Parameters.FLASH_MODE_OFF);
            mCamera.setParameters(parameter);
        }
    }

    @Override
    public boolean isCameraOpened() {
        return mCamera != null;
    }

    @Override
    public void addPictureCallback(PictureCallback pictureCallback) {
        if (pictureCallback != null) {
            if (mPictureCallbackList == null) {
                mPictureCallbackList = new ArrayList<>();
            }
            mPictureCallbackList.add(pictureCallback);
        }
    }

    @Override
    public void addPreviewCallback(PreviewCallback previewCallback) {
        if (previewCallback != null) {
            if (mPreviewCallbackList == null) {
                mPreviewCallbackList = new ArrayList<>();
            }
            mPreviewCallbackList.add(previewCallback);
        }
    }

    @Override
    public void addCameraImpCallback(CameraImpCallback callback) {
        if (callback != null) {
            if (mCameraImpCallbackList == null) {
                mCameraImpCallbackList = new ArrayList<>();
            }
            mCameraImpCallbackList.add(callback);
        }
    }

}
