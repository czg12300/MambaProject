package com.mamba.ui.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.framework.base.BaseFragment;
import com.mamba.R;

/**
 * 消息页面
 *
 * @author jake
 * @since 2017/8/15 上午10:59
 */

public class MessageFragment extends BaseFragment {
    private MessageFragment(){}
    public static MessageFragment newInstance(){
        return new MessageFragment();
    }
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_message,null);
    }
}
