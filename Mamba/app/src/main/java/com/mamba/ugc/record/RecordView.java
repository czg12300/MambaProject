package com.mamba.ugc.record;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.View;

import com.mamba.R;
import com.mamba.ui.widgets.RecordButton;


public class RecordView implements RecordContract.View, View.OnClickListener, RecordButton.Callback {
    private View mContentView;
    private RecordContract.Presenter mPresenter;
    private RecordButton mBtnRecord;
    private GLSurfaceView mGLSurfaceView;

    public RecordView(Context context) {
        mContentView = View.inflate(context, R.layout.layout_record, null);
        initViews();
        setListener();
    }

    private void initViews() {
        mBtnRecord = (RecordButton) mContentView.findViewById(R.id.btn_record);
        mBtnRecord.setCircleColor(0XFFFF6973);
        mGLSurfaceView = (GLSurfaceView) mContentView.findViewById(R.id.gl_camera);
    }

    private void setListener() {
        mBtnRecord.setOnClickListener(this);
        mContentView.findViewById(R.id.btn_toggle).setOnClickListener(this);
    }


    @Override
    public void setPresenter(RecordContract.Presenter presenter) {
        mPresenter = presenter;
    }

    @Override
    public View getView() {
        return mContentView;
    }

    @Override
    public GLSurfaceView getGLSurfaceView() {
        return mGLSurfaceView;
    }

    @Override
    public Context getContext() {
        return mContentView.getContext();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_record:
//                if (v.isSelected()) {
//                    mRecordHolder.stop();
//                    v.setSelected(false);
//                    ((TextView) v).setText("开始");
//                } else {
//                    ((TextView) v).setText("结束");
//                    v.setSelected(true);
//                    mRecordHolder.start(speed);
//                }
                break;
            case R.id.btn_toggle:
                mPresenter.onToggleButtonClicked();
                break;
        }
    }

    @Override
    public void onStartRecord() {
        mPresenter.onStartRecord();
    }

    @Override
    public void onEndRecord() {
        mPresenter.onStopRecord();
    }

    @Override
    public boolean preventRecord() {
        return false;
    }
}
