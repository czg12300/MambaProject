package com.framework.base;

import android.support.v4.app.Fragment;
import android.view.View;

/**
 * fragment基类
 *
 * @author jake
 * @since 2017/8/14 下午6:19
 */

public class BaseFragment extends Fragment {
    protected View findViewById(int id) {
        return getView() != null ? getView().findViewById(id) : null;
    }
}
