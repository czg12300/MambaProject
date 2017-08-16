package com.mamba.ui.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.framework.base.BaseFragment;
import com.mamba.R;

/**
 * 首页视频列表页面
 *
 * @author jake
 * @since 2017/8/15 上午9:57
 */

public class HomeVideoListFragment extends BaseFragment {
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_video_list,  null);
    }
}
