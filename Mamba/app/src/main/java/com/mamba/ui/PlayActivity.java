package com.mamba.ui;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import com.framework.base.BaseActivity;
import com.mamba.R;
import com.mamba.model.VLog;
import com.mamba.model.play.MediaCodecDecodeTest;
import com.mamba.model.record.RecordHolder;
import com.mamba.model.record.randerer.gpuimage.FilterFactory;
import com.mamba.model.record.randerer.gpuimage.FilterType;
import com.mamba.ui.dialog.LoadingDialog;

import java.io.IOException;

/**
 * 使用mediacodec播放视频
 *
 * @author jake
 * @since 2017/7/18 上午10:23
 */

public class PlayActivity extends BaseActivity implements SurfaceHolder.Callback {
    private MediaCodecDecodeTest decodeTest;
    private String outFile;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play);
        SurfaceView sv = (SurfaceView) findViewById(R.id.gl_camera);
        sv.getHolder().addCallback(this);
        findViewById(R.id.btn_record).setOnClickListener(onClickListener);
        findViewById(R.id.btn_toggle).setOnClickListener(onClickListener);
        outFile = getIntent().getStringExtra("out_file");
        VLog.d("PlayActivity outFile " + outFile);
        decodeTest = new MediaCodecDecodeTest();
        decodeTest.setListener(new MediaCodecDecodeTest.Listener() {
            long start;

            @Override
            public void onStart() {
                start = System.currentTimeMillis();
                showLoadingDialog();

            }

            @Override
            public void onFinish() {
                long now = System.currentTimeMillis();
                VLog.d("cast time is " + (now - start));
                hideLoadingDialog();
            }
        });
    }

    private Activity getThis() {
        return this;
    }

    private View.OnClickListener onClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.btn_record:
                    decodeTest.start(outFile);
                    break;
                case R.id.btn_toggle:
                    break;
            }
        }
    };
    private LoadingDialog loadingDialog;

    private void showLoadingDialog() {
        if (isFinishing()) {
            return;
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (loadingDialog == null) {
                    loadingDialog = new LoadingDialog(getThis());
                }
                loadingDialog.show();
            }
        });
    }

    private void hideLoadingDialog() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (loadingDialog != null) {
                    loadingDialog.dismiss();
                }
            }
        });
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        decodeTest.setSurface(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
