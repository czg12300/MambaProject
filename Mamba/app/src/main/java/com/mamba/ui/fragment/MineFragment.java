package com.mamba.ui.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.framework.base.BaseFragment;
import com.mamba.R;

/**
 * 我的页面
 *
 * @author jake
 * @since 2017/8/15 上午10:59
 */

public class MineFragment extends BaseFragment {
    private MineFragment() {
    }

    public static MineFragment newInstance() {
        return new MineFragment();
    }
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_mine,null);
    }
}
