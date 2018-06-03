package com.jake.mamba.camera;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

/**
 * 使用camera2的api创建和打开摄像头设备进行录制
 *
 * @author jake
 * @since 2017/12/10 上午9:11
 */

public class RecordCamera {
    //后置摄像头id
    private String CAMERA_ID_BACK = "0";
    //前置摄像头id
    private String CAMERA_ID_FRONT = "1";
    private CameraDevice mDevice;
    private CameraManager mCameraManager;
    private CaptureRequest.Builder mPreviewBuilder;
    private ImageReader mPreviewImageReader;
    private CameraCaptureSession mCameraCaptureSession;
    private ArrayList<PreviewCallback> mPreviewCallbackList;
    private ArrayList<Callback> mCallbackList;
    private SurfaceTexture mPreviewSurfaceTexture;
    private SurfaceHolder mPreviewSurfaceHolder;
    private boolean hasAvailableCamera = false;
    private String mCurrentCameraId;
    private int mPreviewFormat = ImageFormat.YV12;
    private int mPreviewWidth = 720;
    private int mPreviewHeight = 1280;
    private Context mAppContext;
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
private Handler mHandler=new Handler(Looper.getMainLooper());
    public RecordCamera(Context context) {
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
            try {
                updateCameraParameters();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mDevice = null;
            if (mCallbackList != null) {
                for (Callback callback : mCallbackList) {
                    if (callback != null) {
                        callback.onClosed();
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

    };

    private void updateCameraParameters() throws CameraAccessException {
        CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(mCurrentCameraId);
        StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        if (map != null) {
//            CameraUtils.Size previewSize = CameraUtils.getLargeSize(previewSizes, mPreviewWidth, mPreviewHeight);
//            mPreviewWidth=previewSize.width;
//            mPreviewHeight=previewSize.height;
        }
        mPreviewBuilder = mDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
        List<Surface> surfaceList = new ArrayList<>();
        if (mPreviewSurfaceTexture != null) {
            mPreviewSurfaceTexture.setDefaultBufferSize(mPreviewWidth, mPreviewHeight);
            Surface textureSurface = new Surface(mPreviewSurfaceTexture);
            surfaceList.add(textureSurface);
            mPreviewBuilder.addTarget(textureSurface);
        }
        if (mPreviewSurfaceHolder != null) {
            surfaceList.add(mPreviewSurfaceHolder.getSurface());
            mPreviewBuilder.addTarget(mPreviewSurfaceHolder.getSurface());
        }
        if (mPreviewCallbackList != null && mPreviewCallbackList.size() > 0) {
            if (mPreviewImageReader == null) {
                mPreviewImageReader = ImageReader.newInstance(mPreviewWidth, mPreviewHeight, mPreviewFormat, 2);
                mPreviewImageReader.setOnImageAvailableListener(mPreviewOnImageAvailableListener, null);
            }
            mPreviewBuilder.addTarget(mPreviewImageReader.getSurface());
            surfaceList.add(mPreviewImageReader.getSurface());
        }
        setZoom(0, false);
        updateAutoFocus(characteristics);
        if (surfaceList != null) {
            mDevice.createCaptureSession(surfaceList, mCaptureSessionStateCallback, mHandler);
        }

    }

    private void updateAutoFocus(CameraCharacteristics characteristics) {
        int[] modes = characteristics.get(CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES);
        // Auto focus is not supported
        if (modes == null || modes.length == 0 || (modes.length == 1 && modes[0] == CameraCharacteristics.CONTROL_AF_MODE_OFF)) {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_OFF);
        } else {
            mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_VIDEO);
        }
    }

    private CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureCompleted( CameraCaptureSession session, CaptureRequest request,  TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
            //TODO 功能扩展
        }
    };
    CameraCaptureSession.StateCallback mCaptureSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured( CameraCaptureSession session) {
            mCameraCaptureSession = session;
            if (mCallbackList != null) {
                for (Callback callback : mCallbackList) {
                    if (callback != null) {
                        callback.onOpened(getThis(), mPreviewWidth, mPreviewHeight);
                    }
                }
            }
        }

        @Override
        public void onConfigureFailed( CameraCaptureSession session) {
            release();
        }
    };

    private RecordCamera getThis() {
        return this;
    }

    public void setParameters(int format, int previewWidth, int previewHeight) {
        mPreviewFormat = format;
        mPreviewWidth = previewWidth;
        mPreviewHeight = previewHeight;
    }

    public void setDisplay(SurfaceTexture surfaceTexture) {
        mPreviewSurfaceTexture = surfaceTexture;
    }

    public void setDisplay(SurfaceHolder holder) {
        mPreviewSurfaceHolder = holder;
    }

    public void openFrontCamera() {
        mCurrentCameraId = CAMERA_ID_FRONT;
        openCamera();
    }

    public void openBackCamera() {
        mCurrentCameraId = CAMERA_ID_BACK;
        openCamera();
    }

    public void toggleCamera() {
        if (TextUtils.equals(mCurrentCameraId, CAMERA_ID_FRONT)) {
            openBackCamera();
        } else {
            openFrontCamera();
        }
    }

    @SuppressLint("MissingPermission")
    private void openCamera() {
        if (!hasAvailableCamera) {

            return;
        }
//        if (ActivityCompat.checkSelfPermission(mAppContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
//            return;
//        }
        stopCamera();
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            mCameraManager.openCamera(mCurrentCameraId, mStateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
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
            if (mCameraCaptureSession != null && mPreviewBuilder != null) {
                mCameraCaptureSession.setRepeatingRequest(mPreviewBuilder.build(), mCaptureCallback, mHandler);
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
        if (mCameraCaptureSession != null) {
            mCameraCaptureSession.close();
            mCameraCaptureSession = null;
        }
        if (mDevice != null) {
            mDevice.close();
            mDevice = null;
        }
        if (mPreviewImageReader != null) {
            mPreviewImageReader.close();
            mPreviewImageReader = null;
        }
    }

    public void release() {
        stopCamera();
        if (mCallbackList != null) {
            mCallbackList.clear();
            mCallbackList = null;
        }
        if (mPreviewCallbackList != null) {
            mPreviewCallbackList.clear();
            mPreviewCallbackList = null;
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
//                    mPreviewBuilder.set(CaptureRequest.CONTROL_AE_MODE, CameraMetadata.CONTROL_AE_MODE_OFF);
                    mPreviewBuilder.set(CaptureRequest.LENS_FOCUS_DISTANCE, Integer.valueOf(zoom).floatValue());
                }
//                updateAutoFocus(characteristics);
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


    public void addPreviewCallback(PreviewCallback previewCallback) {
        if (previewCallback != null) {
            if (mPreviewCallbackList == null) {
                mPreviewCallbackList = new ArrayList<>();
            }
            mPreviewCallbackList.add(previewCallback);
        }
    }

    public void addCallback(Callback callback) {
        if (callback != null) {
            if (mCallbackList == null) {
                mCallbackList = new ArrayList<>();
            }
            mCallbackList.add(callback);
        }
    }

    private ImageReader.OnImageAvailableListener mPreviewOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            try (Image image = reader.acquireNextImage()) {
                Image.Plane[] planes = image.getPlanes();
                if (planes.length > 0) {
                    ByteBuffer buffer = planes[0].getBuffer();
                    byte[] data = new byte[buffer.remaining()];
                    buffer.get(data);
                    if (mPreviewCallbackList != null) {
                        for (PreviewCallback callback : mPreviewCallbackList) {
                            if (callback != null) {
                                callback.onPreviewFrame(data, mPreviewWidth, mPreviewHeight, getThis());
                            }
                        }
                    }
                }
            }
        }
    };

    public interface Callback {
        void onOpened(RecordCamera camera, int width, int height);

        void onClosed();
    }

    /**
     * 预览帧回调，结果可能不在主线程
     */
    public interface PreviewCallback {
        void onPreviewFrame(byte[] data, int width, int height, RecordCamera camera);
    }
}
