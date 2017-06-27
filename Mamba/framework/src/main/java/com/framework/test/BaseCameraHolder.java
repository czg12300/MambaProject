package com.framework.test;

import android.content.res.Configuration;
import android.hardware.Camera;
import android.view.Surface;

import java.util.ArrayList;
import java.util.List;

/**
 * 摄像头使用帮助基类
 *
 * @author jake
 * @since 2017/2/9 下午4:51
 */

public abstract class BaseCameraHolder {
    protected static final int DEFAULT_PICTURE_WIDTH = 720;
    protected static final int DEFAULT_PICTURE_HEIGHT = 1280;
    /**
     * 前置摄像头id
     */
    public static final int CAMERA_ID_FRONT = 1;
    /**
     * 后置摄像头id
     */
    public static final int CAMERA_ID_BACK = 0;
    protected Camera mCamera;
    protected int mPositionCameraId = 0;
    private Camera.Parameters mCameraParameters = null;
    protected Camera.ShutterCallback mShutterCallback;
    protected Camera.PictureCallback mRawCallback;
    protected Camera.PictureCallback mPostviewCallback;
    protected Camera.PictureCallback mJpegCallback;
    protected Camera.PreviewCallback mPreviewCallback;
    private int mRotation = 1;
    private ArrayList<CameraListener> mCameraListenerList;

    public void addCameraListener(CameraListener listener) {
        if (listener == null) {
            return;
        }
        if (mCameraListenerList == null) {
            mCameraListenerList = new ArrayList<>();
        }
        mCameraListenerList.add(listener);
    }

    public void removeCameraListener(CameraListener listener) {
        if (listener == null || mCameraListenerList == null) {
            return;
        }
        mCameraListenerList.remove(listener);
    }

    public void setRotation(int rotation) {
        mRotation = rotation;
    }

    public void setPreviewCallback(Camera.PreviewCallback callback) {
        mPreviewCallback = callback;
    }

    public void setShutterCallback(Camera.ShutterCallback callback) {
        mShutterCallback = callback;
    }

    public void setRawPictureCallback(Camera.PictureCallback callback) {
        mRawCallback = callback;
    }

    public void setPostviewPictureCallback(Camera.PictureCallback callback) {
        mPostviewCallback = callback;
    }

    public void setJpegPictureCallback(Camera.PictureCallback callback) {
        mJpegCallback = callback;
    }

    public void reset() {
        stopPreviewAndRelease();
        openCamera(mPositionCameraId);
    }

    public void openCamera(int cameraId) {
        stopPreviewAndRelease();
        mCamera = Camera.open(cameraId);
        notifyCameraOpened();
        if (mCameraParameters == null) {
            mCameraParameters = getDefaultParameters();
        }
        int displayDegree=   getCameraDisplayOrientation();
        if(mRotation== Configuration.ORIENTATION_PORTRAIT){
            mCameraParameters.set("orientation", "portrait");
        }else{
            mCameraParameters.set("orientation", "landscape");
        }
        mCamera.setParameters(mCameraParameters);
        mCamera.setDisplayOrientation(displayDegree);
        if (mPreviewCallback != null) {
            mCamera.setPreviewCallback(mPreviewCallback);
        }
        setCameraPreviewDisplay(mCamera);

    }

    private void notifyCameraOpened() {
        if (mCameraListenerList != null && mCameraListenerList.size() > 0) {
            for (CameraListener listener : mCameraListenerList) {
                if (listener != null) {
                    listener.onCameraOpened(mCamera);
                }
            }
        }
    }

    public void stopPreviewAndRelease() {
        if (isCameraOpen()) {
            mCamera.setPreviewCallback(null);
            mCamera.stopPreview();//停掉原来摄像头的预览
            mCamera.release();//释放资源
            mCamera = null;//取消原来摄像头
        }
    }

    public void autoFocus() {
        if (isCameraOpen()) {
            mCamera.autoFocus(null);
        }
    }

    public void autoFocus(Camera.AutoFocusCallback callback) {
        if (isCameraOpen()) {
            mCamera.autoFocus(callback);
        }
    }

    public void takePicture() {
        mCamera.takePicture(mShutterCallback, mRawCallback, mPostviewCallback, mJpegCallback);
    }

    public void takePictureWithAutoFocus() {
        autoFocus(new Camera.AutoFocusCallback() {
            @Override
            public void onAutoFocus(boolean success, Camera camera) {
                if (success) {
                    mCamera.takePicture(mShutterCallback, mRawCallback, mPostviewCallback, mJpegCallback);
                }
            }
        });

    }

    public Camera getCamera() {
        return mCamera;
    }

    public void stopPreview() {
        if (isCameraOpen()) {
            mCamera.stopPreview();//停掉原来摄像头的预览
        }
    }

    public void startPreview() {
        if (isCameraOpen()) {
            mCamera.startPreview();
        }
    }

    public void toggleCamera() {
        mPositionCameraId = (mPositionCameraId == CAMERA_ID_FRONT) ? CAMERA_ID_BACK : CAMERA_ID_FRONT;
        openCamera(mPositionCameraId);
        startPreview();
    }

    public void setCameraParameters(Camera.Parameters parameter) {
        mCameraParameters = parameter;
        if (isCameraOpen()) {
            mCamera.setParameters(mCameraParameters);
        }
    }

    public Camera.Parameters getDefaultParameters() {
        if (isCameraOpen()) {
            Camera.Parameters parameters = mCamera.getParameters();
            if (parameters.getSupportedFocusModes()
                    .contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE)) {
                parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            }
            List<Camera.Size> preSizes = parameters.getSupportedPreviewSizes();
            Camera.Size largeSize = getLargeSize(preSizes, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
            parameters.setPreviewSize(largeSize.width, largeSize.height);
            List<Camera.Size> pictureSizes = parameters.getSupportedPictureSizes();
            Camera.Size picSize = getLargeSize(pictureSizes, DEFAULT_PICTURE_WIDTH, DEFAULT_PICTURE_HEIGHT);
            parameters.setPictureSize(picSize.width, picSize.height);
//            parameters.setPreviewFrameRate(25);
            return parameters;
        }
        return null;
    }

    public void changeZoom(int zoom) {
        if (isCameraOpen()) {
            Camera.Parameters parameters = mCamera.getParameters();
            if (parameters.isZoomSupported()) {
                parameters.setZoom(zoom);
                mCamera.setParameters(parameters);
            }
        }
    }

    public int getZoom() {
        int ret = 0;
        if (isCameraOpen()) {
            Camera.Parameters parameters = mCamera.getParameters();
            ret = parameters.getZoom();
        }
        return ret;
    }

    public int getMaxZoom() {
        int ret = 0;
        if (isCameraOpen()) {
            Camera.Parameters parameters = mCamera.getParameters();
            ret = parameters.getMaxZoom();
        }
        return ret;
    }

    /**
     * Android API: Display Orientation Setting
     * Just change screen display orientation,
     * the rawFrame data never be changed.
     */
    private int getCameraDisplayOrientation() {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(mPositionCameraId, info);
        int degrees = 0;
        switch (mRotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;
            case Surface.ROTATION_90:
                degrees = 90;
                break;
            case Surface.ROTATION_180:
                degrees = 180;
                break;
            case Surface.ROTATION_270:
                degrees = 270;
                break;
        }
        int displayDegree;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            displayDegree = (info.orientation + degrees) % 360;
            displayDegree = (360 - displayDegree) % 360;  // compensate the mirror
        } else {
            displayDegree = (info.orientation - degrees + 360) % 360;
        }
        return displayDegree;
    }

    protected abstract void setCameraPreviewDisplay(Camera camera);

    private boolean isCameraOpen() {
        return mCamera != null;
    }

    /**
     * 1.找出和屏幕宽高都一样的，如果有直接返回
     * 2.找出所有比率相等的，取出和参考宽度最相近的一个
     * 3.如果没有比率相等的，找出比率差距在0.1内的，取出和参考宽度最相近的一个
     * 4.如果比率相同的最大size的宽度小于屏幕宽高的2/3,则查看0.1比率的最大值
     * 5.如果0.1比率的最大size的宽度也小于屏幕宽高的2/3，取第一个 6.如果没有，取第一个
     *
     * @param list   被查找的数组
     * @param width  参考的宽度
     * @param height 参考用的高度
     * @return
     */
    private Camera.Size getLargeSize(List<Camera.Size> list, int width, int height) {
        if (width > height) {
            int tempwidth = width;
            width = height;
            height = tempwidth;
        }
        // 存放宽高与屏幕宽高相同的size
        Camera.Size size = null;
        // 存放比率相同的最大size
        Camera.Size largeSameRatioSize = null;
        // 存放比率差距0.1的最大size
        Camera.Size largeRatioSize = null;
        float scrwhRatio = width * 1.0f / height * 1.0f;
        for (Camera.Size preSize : list) {
            float tempRatio = preSize.width * 1.0f / preSize.height * 1.0f;
            if (preSize.width < preSize.height) {
                tempRatio = preSize.width * 1.0f / preSize.height * 1.0f;
                if (preSize.width == width && preSize.height == height) {
                    size = preSize;
                    break;
                }
            } else if (preSize.width > preSize.height) {
                tempRatio = preSize.height * 1.0f / preSize.width * 1.0f;
                if (preSize.height == width && preSize.width == height) {
                    size = preSize;
                    break;
                }
            }

            if (tempRatio == scrwhRatio) {
                if (largeSameRatioSize == null) {
                    largeSameRatioSize = preSize;
                } else {
                    if (Math.abs(largeSameRatioSize.width - width) > Math.abs(preSize.width - width)) {
                        largeSameRatioSize = preSize;
                    }
                }
            }

            float ratioDistance = Math.abs(tempRatio - scrwhRatio);
            if (ratioDistance < 0.1) {
                if (largeRatioSize == null) {
                    largeRatioSize = preSize;
                } else {
                    if (Math.abs(largeRatioSize.width - width) > Math.abs(preSize.width - width)) {
                        largeRatioSize = preSize;
                    }
                }
            }
        }

        if (size != null) {
            return size;
        } else if (largeSameRatioSize != null) {
            if (Math.abs(largeSameRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                return largeSameRatioSize;
            } else if (largeRatioSize != null) {
                if (Math.abs(largeRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                    return largeRatioSize;
                } else {
                    return list.get(0);
                }
            } else {
                return list.get(0);
            }
        } else if (largeRatioSize != null) {
            if (Math.abs(largeRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                return largeRatioSize;
            } else {
                return list.get(0);
            }
        } else {
            return list.get(0);
        }
    }

    /**
     * 摄像头监听
     */
    public static interface CameraListener {
        void onCameraOpened(Camera camera);
    }
}
