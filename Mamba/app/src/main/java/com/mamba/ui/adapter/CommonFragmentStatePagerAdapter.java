package com.mamba.ui.adapter;

import android.os.Parcelable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentStatePagerAdapter;
import android.view.ViewGroup;

import com.mamba.ui.entity.FragmentPage;

import java.util.ArrayList;
import java.util.List;

/**
 * 游戏页ViewPager适配器
 *
 * @author jake
 * @since 2016/7/23 16:33
 */
public class CommonFragmentStatePagerAdapter extends FragmentStatePagerAdapter {
    private List<FragmentPage> list;

    public CommonFragmentStatePagerAdapter(FragmentManager fm) {
        this(fm, null);
    }

    public CommonFragmentStatePagerAdapter(FragmentManager fm, List<FragmentPage> list) {
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

    @Override
    public void destroyItem(ViewGroup container, int position, Object object) {
    }

    /**
     * 在调用notifyDataSetChanged()方法后，随之会触发该方法，根据该方法返回的值来确定是否更新
     * object对象为Fragment，具体是当前显示的Fragment和它的前一个以及后一个
     */
    @Override
    public int getItemPosition(Object object) {
        return POSITION_NONE;   // 返回发生改变，让系统重新加载
    }

    @Override
    public Parcelable saveState() {
        return null;
    }

    public void release() {
        if (list != null) {
            list.clear();
        }
    }
}