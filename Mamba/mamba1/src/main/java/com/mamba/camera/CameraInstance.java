package com.mamba.camera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.MeteringRectangle;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.util.Log;
import android.util.Range;
import android.util.Size;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.WindowManager;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

public class CameraInstance implements ICameraRenderer {
    //后置摄像头id
    private String CAMERA_ID_BACK = "0";
    //前置摄像头id
    private String CAMERA_ID_FRONT = "1";
    private CameraDevice mDevice;
    private CameraManager mCameraManager;
    private CaptureRequest.Builder mPreviewBuilder;
    private ImageReader mPictureImageReader;
    private ImageReader mPreviewImageReader;
    private CameraCaptureSession mCameraCaptureSession;
    private ArrayList<ImageReader.OnImageAvailableListener> mPictureCallbackList;
    private ArrayList<ImageReader.OnImageAvailableListener> mPreviewCallbackList;
    private ArrayList<CameraCallback> mCameraCallbackList;
    private SurfaceTexture mPreviewSurfaceTexture;
    private SurfaceHolder mPreviewSurfaceHolder;
    private boolean hasAvailableCamera = false;
    private String mCurrentCameraId;
    private volatile Size mPreviewSize = new Size(1080, 1920);
    private volatile Size mPictureSize = new Size(1080, 1920);
    private int mPictureFormat = ImageFormat.JPEG;
    private int mPreviewFormat = ImageFormat.YUV_420_888;
    private Context mAppContext;
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
    private Handler mListenerHandler;
    private CameraCharacteristics mCameraCharacteristics;
    private int mCaptureTemplate = CameraDevice.TEMPLATE_RECORD;

    public CameraInstance(Context context) {
        mAppContext = context.getApplicationContext();
        mCameraManager = (CameraManager) mAppContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            initCameraId();
        } catch (CameraAccessException e) {
            e.printStackTrace();
            hasAvailableCamera = false;
        }
    }

    private void initCameraId() throws CameraAccessException {
        String[] ids = mCameraManager.getCameraIdList();
        if (ids != null && ids.length > 0) {
            hasAvailableCamera = true;
            for (String id : ids) {
                CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(id);
                Integer level = characteristics.get(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
                if (level == null ||
                        level == CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY) {
                    continue;
                }
                int internal = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (internal == CameraCharacteristics.LENS_FACING_BACK) {
                    CAMERA_ID_BACK = id;
                } else if (internal == CameraCharacteristics.LENS_FACING_FRONT) {
                    CAMERA_ID_FRONT = id;
                }
            }
        }
        mCurrentCameraId = CAMERA_ID_BACK;
    }

    private CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(CameraDevice cameraDevice) {
            mDevice = cameraDevice;
            mCameraOpenCloseLock.release();
            Log.d("tag", "mCaptureTemplate " + mCaptureTemplate);
            try {
                mPreviewBuilder = mDevice.createCaptureRequest(mCaptureTemplate);
                mCameraCharacteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                createCaptureSession();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mDevice = null;
            if (mCameraCallbackList != null) {
                for (CameraCallback callback : mCameraCallbackList) {
                    if (callback != null) {
                        callback.onCameraClosed();
                    }
                }
            }
        }

        @Override
        public void onError(CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mDevice = null;
        }

        private void createCaptureSession() {
            StreamConfigurationMap map = mCameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            if (map != null) {
                Size[] supportSizeUps = map.getOutputSizes(ImageFormat.JPEG);
                Size[] supportSizeLows = map.getOutputSizes(ImageFormat.YUV_420_888);
                int presetPreviewSize = mPreviewSize.getWidth() * mPreviewSize.getHeight();
                int presetPictureSize = mPictureSize.getWidth() * mPictureSize.getHeight();
                int sizeLow = supportSizeLows[0].getWidth() * supportSizeLows[0].getHeight();
                mPreviewSize = getLargeSize(presetPreviewSize > sizeLow ? supportSizeUps : supportSizeLows, mPreviewSize.getWidth(), mPreviewSize.getHeight());
                mPictureSize = getLargeSize(presetPictureSize > sizeLow ? supportSizeUps : supportSizeLows, mPictureSize.getWidth(), mPictureSize.getHeight());
            }
            List<Surface> surfaceList = new ArrayList<>();
            if (mPreviewSurfaceTexture != null) {
                mPreviewSurfaceTexture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
                Surface textureSurface = new Surface(mPreviewSurfaceTexture);
                surfaceList.add(textureSurface);
                mPreviewBuilder.addTarget(textureSurface);
            }
            if (mPreviewSurfaceHolder != null) {
                surfaceList.add(mPreviewSurfaceHolder.getSurface());
                mPreviewBuilder.addTarget(mPreviewSurfaceHolder.getSurface());
            }
            if (mPictureCallbackList != null && mPictureCallbackList.size() > 0) {
                if (mPictureImageReader != null) {
                    mPictureImageReader.close();
                }
                mPictureImageReader = ImageReader.newInstance(mPictureSize.getWidth(), mPictureSize.getHeight(), mPictureFormat, 1);
                mPictureImageReader.setOnImageAvailableListener(mPictureOnImageAvailableListener, mListenerHandler);
                surfaceList.add(mPictureImageReader.getSurface());
                mPreviewBuilder.addTarget(mPictureImageReader.getSurface());
            }
            if (mPreviewCallbackList != null && mPreviewCallbackList.size() > 0) {
                if (mPreviewImageReader != null) {
                    mPreviewImageReader.close();
                }
                mPreviewImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), mPreviewFormat, 1);
                mPreviewImageReader.setOnImageAvailableListener(mPreviewOnImageAvailableListener, mListenerHandler);
                surfaceList.add(mPreviewImageReader.getSurface());
                mPreviewBuilder.addTarget(mPreviewImageReader.getSurface());
            }
            if (surfaceList != null) {
                try {
                    mDevice.createCaptureSession(surfaceList, mCaptureSessionStateCallback, mListenerHandler);
                } catch (CameraAccessException e) {
                    e.printStackTrace();
                }
            }

        }
    };


    /**
     * 设置预览的模板
     *
     * @param template CameraDevice.TEMPLATE_PREVIEW
     */
    public void setCaptureTemplate(int template) {
        mCaptureTemplate = template;
        if (isCameraOpened()) {
            openCamera();
        }
    }

    private void updateAutoFocus() {
        int[] modes = mCameraCharacteristics.get(CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES);
        // Auto focus is not supported
        if (modes == null || modes.length == 0 || (modes.length == 1 && modes[0] == CameraCharacteristics.CONTROL_AF_MODE_OFF)) {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_OFF);
        } else {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO);
        }
    }

    private CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);

            //TODO 功能扩展
        }
    };
    CameraCaptureSession.StateCallback mCaptureSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mCameraCaptureSession = session;
            if (mCameraCallbackList != null) {
                for (CameraCallback callback : mCameraCallbackList) {
                    if (callback != null) {
                        callback.onCameraOpened(getCameraInstance(), mPreviewSize.getWidth(), mPreviewSize.getHeight());
                    }
                }
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            release();
        }
    };

    public void setPreviewFormat(int format) {
        mPreviewFormat = format;
    }

    public void setPictureFormat(int format) {
        mPictureFormat = format;
    }

    public void setPictureSize(int width, int height) {
        mPictureSize = new Size(width, height);
    }

    public void setPreviewSize(int width, int height) {
        mPreviewSize = new Size(width, height);
    }

    private int getRotateDegree() {
        if (!isCameraOpened()) {
            return 0;
        }
        Display d = ((WindowManager) mAppContext.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
        int displayRotation = d.getRotation();
        int degrees = 0;
        switch (displayRotation) {
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

        //noinspection ConstantConditions
        int senseOrientation = mCameraCharacteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        return (senseOrientation - degrees + 360) % 360;
    }

    @Deprecated
    public void focusOnTouch(float x, float y, int viewWidth, int viewHeight) {
        if (!isCameraOpened()) {
            return;
        }
        // 先取相对于view上面的坐标
        int displayRotate = getRotateDegree();
        int realPreviewWidth = mPreviewSize.getWidth();
        int realPreviewHeight = mPreviewSize.getHeight();
        if (90 == displayRotate || 270 == displayRotate) {
            realPreviewWidth = mPreviewSize.getHeight();
            realPreviewHeight = mPreviewSize.getWidth();
        }
        // 计算摄像头取出的图像相对于view放大了多少，以及有多少偏移
        float imgScale = 1.0f;
        float verticalOffset = 0;
        float horizontalOffset = 0;
        if (realPreviewHeight * viewWidth > realPreviewWidth * viewHeight) {
            imgScale = viewWidth * 1.0f / realPreviewWidth;
            verticalOffset = (realPreviewHeight - viewHeight / imgScale) / 2;
        } else {
            imgScale = viewHeight * 1.0f / realPreviewHeight;
            horizontalOffset = (realPreviewWidth - viewWidth / imgScale) / 2;
        }
        // 将点击的坐标转换为图像上的坐标
        x = x / imgScale + horizontalOffset;
        y = y / imgScale + verticalOffset;
        if (90 == displayRotate) {
            float tmp = x;
            x = y;
            y = mPreviewSize.getHeight() - tmp;
        } else if (270 == displayRotate) {
            float tmp = x;
            x = mPreviewSize.getWidth() - y;
            y = tmp;
        }

        // 计算取到的图像相对于裁剪区域的缩放系数，以及位移
        Rect cropRegion = mPreviewBuilder.get(CaptureRequest.SCALER_CROP_REGION);
        if (null == cropRegion) {
            cropRegion = new Rect(0, 0, 1, 1);
        }
        int cropWidth = cropRegion.width(), cropHeight = cropRegion.height();
        if (mPreviewSize.getHeight() * cropWidth > mPreviewSize.getWidth() * cropHeight) {
            imgScale = cropHeight * 1.0f / mPreviewSize.getHeight();
            verticalOffset = 0;
            horizontalOffset = (cropWidth - imgScale * mPreviewSize.getWidth()) / 2;
        } else {
            imgScale = cropWidth * 1.0f / mPreviewSize.getWidth();
            horizontalOffset = 0;
            verticalOffset = (cropHeight - imgScale * mPreviewSize.getHeight()) / 2;
        }
        // 将点击区域相对于图像的坐标，转化为相对于成像区域的坐标
        x = x * imgScale + horizontalOffset + cropRegion.left;
        y = y * imgScale + verticalOffset + cropRegion.top;

        double tapAreaRatio = 0.1;
        Rect rect = new Rect();
//        rect.left = clamp((int) (x - tapAreaRatio / 2 * cropRegion.width()), 0, cropRegion.width());
//        rect.right = clamp((int) (x + tapAreaRatio / 2 * cropRegion.width()), 0, cropRegion.width());
//        rect.top = clamp((int) (y - tapAreaRatio / 2 * cropRegion.height()), 0, cropRegion.height());
//        rect.bottom = clamp((int) (y + tapAreaRatio / 2 * cropRegion.height()), 0, cropRegion.height());
        rect.left = 100;
        rect.right = 500;
        rect.top = 500;
        rect.bottom = 1000;
        Log.d("tag", "onTouch " + rect.toString());

        mPreviewBuilder.set(CaptureRequest.CONTROL_AF_REGIONS, new MeteringRectangle[]{new MeteringRectangle(rect, 1000)});
        mPreviewBuilder.set(CaptureRequest.CONTROL_AE_REGIONS, new MeteringRectangle[]{new MeteringRectangle(rect, 1000)});
        mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_AUTO);
        mPreviewBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_START);
        mPreviewBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER, CameraMetadata.CONTROL_AE_PRECAPTURE_TRIGGER_START);

        try {
            mCameraCaptureSession.setRepeatingRequest(mPreviewBuilder.build(), mCaptureCallback, mListenerHandler);
        } catch (CameraAccessException e) {
        }
    }

    private int clamp(int x, int min, int max) {
        if (x > max) return max;
        if (x < min) return min;
        return x;
    }

    public void setExposure(int ae) {
        if (isCameraOpened()) {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION, ae);
        }
    }

    public int getExposure() {
        if (isCameraOpened()) {
            return mPreviewBuilder.get(CaptureRequest.CONTROL_AE_EXPOSURE_COMPENSATION);
        }
        return 0;
    }

    public int getExposureMax() {
        if (isCameraOpened()) {
            Range<Integer> range = mCameraCharacteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
            return range.getUpper();
        }
        return 0;
    }

    public int getExposureMin() {
        if (isCameraOpened()) {
            Range<Integer> range = mCameraCharacteristics.get(CameraCharacteristics.CONTROL_AE_COMPENSATION_RANGE);
            return range.getLower();
        }
        return 0;
    }

    public void setDisplay(SurfaceTexture surfaceTexture) {
        mPreviewSurfaceTexture = surfaceTexture;
    }

    public void setDisplay(SurfaceHolder holder) {
        mPreviewSurfaceHolder = holder;
    }

    public void openFrontCamera() {
        mCurrentCameraId = CAMERA_ID_FRONT;
        handleOpenCamera();
    }

    public void presetCameraBack() {
        mCurrentCameraId = CAMERA_ID_BACK;
    }

    public void presetCameraFront() {
        mCurrentCameraId = CAMERA_ID_FRONT;
    }

    /**
     * 打开当前相机
     */
    public void openCamera() {
        Log.d("tag", "mCurrentCameraId " + mCurrentCameraId);
        handleOpenCamera();
    }

    public void openBackCamera() {
        mCurrentCameraId = CAMERA_ID_BACK;
        handleOpenCamera();
    }

    public void toggleCamera() {
        if (TextUtils.equals(mCurrentCameraId, CAMERA_ID_FRONT)) {
            openBackCamera();
        } else {
            openFrontCamera();
        }
    }

    private void handleOpenCamera() {
        if (!hasAvailableCamera) {
            return;
        }
        if (ActivityCompat.checkSelfPermission(mAppContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        if (mListenerHandler == null) {
            HandlerThread thread = new HandlerThread(CameraInstance.class.getName());
            thread.start();
            mListenerHandler = new Handler(thread.getLooper());
        }
        stopCamera();
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            mCameraManager.openCamera(mCurrentCameraId, mStateCallback, mListenerHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    public void takePicture() {
        if (!isCameraOpened()) {
            return;
        }
        try {
            mPreviewBuilder = mDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            mCameraCaptureSession.stopRepeating();
            mCameraCaptureSession.capture(mPreviewBuilder.build(), mCaptureCallback, mListenerHandler);
        } catch (CameraAccessException e) {
        }
    }

    public boolean isFacingFront() {
        return TextUtils.equals(mCurrentCameraId, CAMERA_ID_FRONT);
    }


    public void startPreview() {
        if (!isCameraOpened()) {
            return;
        }
        try {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AWB_MODE, CaptureRequest.CONTROL_AWB_MODE_AUTO);
            updateAutoFocus();
            if (mCameraCaptureSession != null && mPreviewBuilder != null) {
                mCameraCaptureSession.setRepeatingRequest(mPreviewBuilder.build(), mCaptureCallback, mListenerHandler);
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }

    public void stopPreview() {
        if (mCameraCaptureSession != null) {
            try {
                mCameraCaptureSession.stopRepeating();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
    }

    private void stopCamera() {
        stopPreview();
        if (mCameraCaptureSession != null) {
            mCameraCaptureSession.close();
            mCameraCaptureSession = null;
        }
        if (mDevice != null) {
            mDevice.close();
            mDevice = null;
        }
        if (mPictureImageReader != null) {
            mPictureImageReader.close();
            mPictureImageReader = null;
        }
        if (mPreviewImageReader != null) {
            mPreviewImageReader.close();
            mPreviewImageReader = null;
        }
    }

    public void release() {
        stopCamera();
        if (mCameraCallbackList != null) {
            mCameraCallbackList.clear();
            mCameraCallbackList = null;
        }
        if (mPictureCallbackList != null) {
            mPictureCallbackList.clear();
            mPictureCallbackList = null;
        }
        if (mPreviewCallbackList != null) {
            mPreviewCallbackList.clear();
            mPreviewCallbackList = null;
        }
        if (mListenerHandler != null) {
            mListenerHandler.getLooper().quit();
            mListenerHandler = null;
        }
    }

    public int getMaxZoom() {
        if (isCameraOpened()) {
            try {
                CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                Float distance = characteristics.get(CameraCharacteristics.LENS_INFO_MINIMUM_FOCUS_DISTANCE);
                if (distance != null) {
                    return distance.intValue();
                }
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
        return 0;
    }

    public void setZoom(int zoom) {
        setZoom(zoom, true);
    }

    private void setZoom(int zoom, boolean isStartPreview) {
        if (isCameraOpened()) {
            try {
                CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                Float distance = characteristics.get(CameraCharacteristics.LENS_INFO_MINIMUM_FOCUS_DISTANCE);
                if (distance != null) {
                    if (zoom > distance.intValue()) {
                        zoom = distance.intValue();
                    } else if (zoom < 0) {
                        zoom = 0;
                    }
                    mPreviewBuilder.set(CaptureRequest.LENS_FOCUS_DISTANCE, Integer.valueOf(zoom).floatValue());
                }
                updateAutoFocus();
                if (isStartPreview) {
                    startPreview();
                }
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
    }

    public int getZoom() {
        if (isCameraOpened()) {
            try {
                CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
                Float distance = characteristics.get(CameraCharacteristics.LENS_INFO_MINIMUM_FOCUS_DISTANCE);
                if (distance != null) {
                    distance = mPreviewBuilder.get(CaptureRequest.LENS_FOCUS_DISTANCE);
                    return distance.intValue();
                }
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
        return 0;
    }

    public boolean zoomIn() {
        if (isCameraOpened()) {
            int max = getMaxZoom();
            if (max > 0) {
                int zoom = getZoom();
                zoom += max / 10;
                if (zoom > max) {
                    zoom = max;
                }
                setZoom(zoom, true);
                return true;
            }

        }
        return false;
    }

    public boolean zoomOut() {
        if (isCameraOpened()) {
            int max = getMaxZoom();
            if (max > 0) {
                int zoom = getZoom();
                zoom -= max / 10;
                if (zoom < 0) {
                    zoom = 0;
                }
                setZoom(zoom, true);
                return true;
            }

        }
        return false;
    }

    public void openFlashLight() {
        if (mPreviewBuilder != null) {
            mPreviewBuilder.set(CaptureRequest.FLASH_MODE, CaptureRequest.FLASH_MODE_TORCH);
            startPreview();
        }

    }

    public void closeFlashLight() {
        if (mPreviewBuilder != null) {
            mPreviewBuilder.set(CaptureRequest.FLASH_MODE, CaptureRequest.FLASH_MODE_OFF);
            startPreview();
        }
    }

    public boolean isCameraOpened() {
        return mDevice != null;
    }


    public void addPictureCallback(ImageReader.OnImageAvailableListener pictureCallback) {
        if (pictureCallback != null) {
            if (mPictureCallbackList == null) {
                mPictureCallbackList = new ArrayList<>();
            }
            mPictureCallbackList.add(pictureCallback);
        }
    }

    public void addCameraCallback(CameraCallback callback) {
        if (callback != null) {
            if (mCameraCallbackList == null) {
                mCameraCallbackList = new ArrayList<>();
            }
            mCameraCallbackList.add(callback);
        }
    }

    public void addPreviewCallback(ImageReader.OnImageAvailableListener previewCallback) {
        if (previewCallback != null) {
            if (mPreviewCallbackList == null) {
                mPreviewCallbackList = new ArrayList<>();
            }
            mPreviewCallbackList.add(previewCallback);
        }
    }


    private ImageReader.OnImageAvailableListener mPictureOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (mPictureCallbackList != null) {
                for (ImageReader.OnImageAvailableListener callback : mPictureCallbackList) {
                    if (callback != null) {
                        callback.onImageAvailable(reader);
                    }
                }
            }

        }
    };
    private ImageReader.OnImageAvailableListener mPreviewOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            if (mPreviewCallbackList != null) {
                for (ImageReader.OnImageAvailableListener callback : mPreviewCallbackList) {
                    if (callback != null) {
                        callback.onImageAvailable(reader);
                    }
                }
            }
        }
    };

    private CameraInstance getCameraInstance() {
        return this;
    }

    private Size getLargeSize(android.util.Size[] list, int width, int height) {
        if (list == null || list.length == 0) {
            return new Size(width, height);
        }
        if (width > height) {
            int tempwidth = width;
            width = height;
            height = tempwidth;
        }
        // 存放宽高与屏幕宽高相同的size
        Size size = null;
        // 存放比率相同的最大size
        Size largeSameRatioSize = null;
        // 存放比率差距0.1的最大size
        Size largeRatioSize = null;
        float scrwhRatio = width * 1.0f / height * 1.0f;
        for (android.util.Size preSize : list) {
            Log.d("tag", preSize.toString());
            float tempRatio = preSize.getWidth() * 1.0f / preSize.getHeight() * 1.0f;
            if (preSize.getWidth() < preSize.getHeight()) {
                tempRatio = preSize.getWidth() * 1.0f / preSize.getHeight() * 1.0f;
                if (preSize.getWidth() == width && preSize.getHeight() == height) {
                    size = preSize;
                    break;
                }
            } else if (preSize.getWidth() > preSize.getHeight()) {
                tempRatio = preSize.getHeight() * 1.0f / preSize.getWidth() * 1.0f;
                if (preSize.getHeight() == width && preSize.getWidth() == height) {
                    size = preSize;
                    break;
                }
            }

            if (tempRatio == scrwhRatio) {
                if (largeSameRatioSize == null) {
                    largeSameRatioSize = preSize;
                } else {
                    if (Math.abs(largeSameRatioSize.getWidth() - width) > Math.abs(preSize.getWidth() - width)) {
                        largeSameRatioSize = preSize;
                    }
                }
            }

            float ratioDistance = Math.abs(tempRatio - scrwhRatio);
            if (ratioDistance < 0.1) {
                if (largeRatioSize == null) {
                    largeRatioSize = preSize;
                } else {
                    if (Math.abs(largeRatioSize.getWidth() - width) > Math.abs(preSize.getWidth() - width)) {
                        largeRatioSize = preSize;
                    }
                }
            }
        }

        if (size != null) {
            return size;
        } else if (largeSameRatioSize != null) {
            if (Math.abs(largeSameRatioSize.getWidth() - width) < (width * 1.0f / 3.0f)) {
                return largeSameRatioSize;
            } else if (largeRatioSize != null) {
                if (Math.abs(largeRatioSize.getWidth() - width) < (width * 1.0f / 3.0f)) {
                    return largeRatioSize;
                } else {
                    return list[0];
                }
            } else {
                return list[0];
            }
        } else if (largeRatioSize != null) {
            if (Math.abs(largeRatioSize.getWidth() - width) < (width * 1.0f / 3.0f)) {
                return largeRatioSize;
            } else {
                return list[0];
            }
        } else {
            return list[0];
        }
    }

    public interface CameraCallback {
        void onCameraOpened(CameraInstance cameraInstance, int width, int height);

        void onCameraClosed();
    }

}
