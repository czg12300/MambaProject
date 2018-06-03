package com.mamba.ugc.record;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import com.framework.base.BaseActivity;
import com.mamba.R;
import com.mamba.model.VLog;

/**
 * 视频录制页面
 *
 * @author jake
 * @since 2017/6/21 下午3:49
 */

public class RecordActivity extends BaseActivity {
    private static final int REQUEST_PERMISSION = 0x01;
    private RecordPresenter mPresenter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_record);
        RecordContract.View view = new RecordView(this);
        setContentView(view.getView());
        mPresenter = new RecordPresenter(view);
        view.setPresenter(mPresenter);

        checkAndRequestPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.READ_EXTERNAL_STORAGE, REQUEST_PERMISSION);
        checkAndRequestPermission(Manifest.permission.CAMERA, REQUEST_PERMISSION);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mPresenter.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mPresenter.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mPresenter.onDestroy();
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

}
