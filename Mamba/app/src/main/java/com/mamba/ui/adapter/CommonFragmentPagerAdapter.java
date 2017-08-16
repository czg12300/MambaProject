package com.mamba.ui.adapter;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

import com.mamba.ui.entity.FragmentPage;

import java.util.ArrayList;
import java.util.List;

/**
 * 通用ViewPager适配器
 *
 * @author jake
 * @since 2016/7/23 14:25
 */
public class CommonFragmentPagerAdapter extends FragmentPagerAdapter {
    private List<FragmentPage> list;

    public CommonFragmentPagerAdapter(FragmentManager fm) {
        this(fm, null);
    }

    public CommonFragmentPagerAdapter(FragmentManager fm, List<FragmentPage> list) {
        super(fm);
        this.list = list;
    }

    public void setData(List<FragmentPage> list) {
        this.list = list;
    }

    @Override
    public Fragment getItem(int position) {
        boolean hasFragment = list != null && position < list.size();
        return hasFragment ? list.get(position).fragment : null;
    }

    @Override
    public CharSequence getPageTitle(int position) {
        boolean hasTitle = list != null && position < list.size();
        return hasTitle ? list.get(position).title : null;
    }

    @Override
    public int getCount() {
        return list != null ? list.size() : 0;
    }

    public void release() {
        if (list != null) {
            list.clear();
        }
    }
}
