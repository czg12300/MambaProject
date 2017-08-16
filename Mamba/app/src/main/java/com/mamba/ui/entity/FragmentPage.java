package com.mamba.ui.entity;

import android.support.v4.app.Fragment;

/**
 * 使用与fragment+viewpage的数据
 *
 * @author jake
 * @since 2017/8/15 上午10:29
 */

public class FragmentPage {
    public Fragment fragment;
    public String title = null;

    public FragmentPage() {
    }

    public FragmentPage(Fragment fragment) {
        this(fragment, null);
    }

    public FragmentPage(Fragment fragment, String title) {
        this.fragment = fragment;
        this.title = title;
    }
}
