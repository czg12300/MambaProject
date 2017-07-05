package com.mamba.ui;

import android.Manifest;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.framework.base.BaseActivity;
import com.mamba.R;
import com.mamba.model.VLog;
import com.mamba.model.record.RecordHolder;
import com.mamba.model.record.randerer.gpuimage.FilterFactory;
import com.mamba.model.record.randerer.gpuimage.FilterType;

/**
 * 视频录制页面
 *
 * @author jake
 * @since 2017/6/21 下午3:49
 */

public class RecordActivity extends BaseActivity {
    private static final int REQUEST_PERMISSION = 0x01;
    private RecordHolder mRecordHolder;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_record);
        checkAndRequestPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.READ_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.CAMERA, REQUEST_PERMISSION);
        findViewById(R.id.btn_record).setOnClickListener(onClickListener);
        findViewById(R.id.btn_toggle).setOnClickListener(onClickListener);
        mRecordHolder = new RecordHolder();
        mRecordHolder.setGlSurfaceView((GLSurfaceView) findViewById(R.id.gl_camera));
        mRecordHolder.setFilter(FilterFactory.getFilter(this, FilterType.BEAUTY));
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION) {
            for (int i = 0; i < grantResults.length; i++) {
                if (grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    VLog.d("onRequestPermissionsResult success " + permissions[i]);
                } else {
//                    ActivityCompat.requestPermissions(this, new String[]{permissions[i]}, REQUEST_PERMISSION);
                    VLog.d("onRequestPermissionsResult fail " + permissions[i]);
                }
            }
        }
    }

    private View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btn_record:
                    if (v.isSelected()) {
                        mRecordHolder.stopEncode();
                        v.setSelected(false);
                        ((TextView) v).setText("开始");
                    } else {
                        ((TextView) v).setText("结束");
                        v.setSelected(true);
                        mRecordHolder.startEncode();
                    }
                    break;
                case R.id.btn_toggle:
                    if (mRecordHolder != null && mRecordHolder.getCameraImp() != null) {
                        mRecordHolder.getCameraImp().toggleCamera();
                    }
                    break;
            }
        }
    };
}
