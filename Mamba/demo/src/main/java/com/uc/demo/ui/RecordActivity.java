package com.uc.demo.ui;

import android.Manifest;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.jake.mamba.camera.CameraInstance;
import com.jake.mamba.renderer.holder.CameraRendererHolder;
import com.uc.demo.BaseActivity;
import com.uc.demo.R;

/**
 * 视频录制页面
 *
 * @author jake
 * @since 2017/6/21 下午3:49
 */

public class RecordActivity extends BaseActivity {
    private static final int REQUEST_PERMISSION = 0x01;
    private TextView mTvFrameRate;
    CameraRendererHolder mCameraRendererHolder;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_record);
        findViewById(R.id.btn_record).setOnClickListener(onClickListener);
        findViewById(R.id.btn_toggle).setOnClickListener(onClickListener);
        checkAndRequestPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.READ_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.CAMERA, REQUEST_PERMISSION);
        mCameraRendererHolder= new CameraRendererHolder((GLSurfaceView) findViewById(R.id.gl_camera));
//        findViewById(R.id.touch).set、OnTouchListener(new View.OnTouchListener() {
//            @Override
//            public boolean onTouch(View v, MotionEvent event) {
//                if (event.getAction() == MotionEvent.ACTION_DOWN) {
//                    Log.d("tag", "onTouch");
//                    mCameraInstance.focusOnTouch(event.getX(), event.getY(), v.getWidth(), v.getHeight());
//                }
//                return true;
//            }
//        });
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION) {
            for (int i = 0; i < grantResults.length; i++) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    Log.d("tag", "onRequestPermissionsResult success " + permissions[i]);
                } else {
                    ActivityCompat.requestPermissions(this, new String[]{permissions[i]}, REQUEST_PERMISSION);
                    Log.d("tag", "onRequestPermissionsResult fail " + permissions[i]);
                }
            }
        }
    }

    private View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btn_record:
//                    if (v.isSelected()) {
//                        mVideoRecordBySurface.stop();
//                        v.setSelected(false);
//                        ((TextView) v).setText("开始");
//                    } else {
//                        ((TextView) v).setText("结束");
//                        v.setSelected(true);
////                        mVideoRecordBySurface.setSpeed(1.0f);
//                        mVideoRecordBySurface.start(VideoCodecParams.
//                                VideoCodecParametersBuilder
//                                .create()
//                                .setBitRate(4 * 1024 * 1024)
//                                .setCodecType(VideoCodecParams.CodecType.H264)
//                                .setOutFile(getOutputDir() + System.currentTimeMillis() + ".h264")
//                                .setWidth(720)
//                                .setHeight(1280)
//                                .setFrameRate(25)
//                                .setKeyIFrameInterval(1)
//                                .setId(System.currentTimeMillis())
//                                .build());
//                    }
                    break;
                case R.id.btn_toggle:
                    CameraInstance.get().toggleCamera();
                    break;
            }
        }
    };

    private String getOutputDir() {
        return Environment.getExternalStorageDirectory().getAbsolutePath() + "/A_Video/";
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mCameraRendererHolder != null) {
            mCameraRendererHolder.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mCameraRendererHolder != null) {
            mCameraRendererHolder.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mCameraRendererHolder != null) {
            mCameraRendererHolder.onDestroy();
        }
        CameraInstance.get().release();
//        mCameraInstance.release();
    }
//    ImageReader.OnImageAvailableListener callback = new ImageReader.OnImageAvailableListener() {
//        @Override
//        public void onImageAvailable(ImageReader reader) {
//            long last = System.currentTimeMillis();
//            Image image = reader.acquireNextImage();
//            image.close();
//            long now = System.currentTimeMillis();
////            Log.d("tag", " 处理时间：" + (now - last));
////            reader.close();
////            Log.d("tag", "CameraRenderer width:height " + reader.getWidth() + ":" + reader.getHeight());
//            printFps();
//        }
//
//        long lastTimes = 0;
//        int count = 0;
//
//
//        /**
//         * 打印fps
//         */
//        private void printFps() {
//            long now = System.currentTimeMillis();
//            if (lastTimes > 0) {
//                if (now - lastTimes >= 1000) {
//                    mTvFrameRate.post(new Runnable() {
//                        @Override
//                        public void run() {
//
//                            mTvFrameRate.setText("帧率：" + count);
//                        }
//                    });
//                    Log.d("tag", "CameraRenderer onPreviewFrame fps:" + count);
//                    lastTimes = now;
//                    count = 0;
//                } else {
//                    count++;
//                }
//            } else {
//                lastTimes = now;
//            }
//        }
//    };
}
