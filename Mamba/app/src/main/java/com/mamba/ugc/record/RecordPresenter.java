package com.mamba.ugc.record;

import com.mamba.model.record.RecordHolder;
import com.mamba.model.record.renderer.gpuimage.FilterFactory;
import com.mamba.model.record.renderer.gpuimage.FilterType;

/**
 * @author jake
 * @since 2018/3/26 下午11:11
 */

public class RecordPresenter implements RecordContract.Presenter {
    private RecordContract.View mView;
    private RecordHolder mRecordHolder;

    public RecordPresenter(RecordContract.View view) {
        mView = view;
        mRecordHolder = new RecordHolder();
        mRecordHolder.setGlSurfaceView(mView.getGLSurfaceView());
        mRecordHolder.setFilter(FilterFactory.getFilter(mView.getContext(), FilterType.BEAUTY));
    }

    @Override
    public void onStartRecord() {
        mRecordHolder.start(RecordHolder.Speed.NORMAL);
    }

    @Override
    public void onStopRecord() {
        mRecordHolder.stop();
    }

    @Override
    public void onResume() {
    }

    @Override
    public void onPause() {

    }

    @Override
    public void onDestroy() {

    }

    @Override
    public void onToggleButtonClicked() {
        mRecordHolder.getCamera().toggleCamera();
    }
}
