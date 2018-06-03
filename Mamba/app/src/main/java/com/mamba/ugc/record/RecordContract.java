package com.mamba.ugc.record;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.view.View;

import com.mamba.common.MvpPresenter;
import com.mamba.common.MvpView;

/**
 * 录制页面
 *
 * @author jake
 * @since 2018/3/26 下午11:08
 */

public interface RecordContract {

    interface View extends MvpView<Presenter> {
        android.view.View getView();
        GLSurfaceView getGLSurfaceView();
        Context getContext();
    }

    interface Presenter extends MvpPresenter {
        void onStartRecord();
        void onStopRecord();
        void onResume();

        void onPause();

        void onDestroy();

        void onToggleButtonClicked();
    }
}
