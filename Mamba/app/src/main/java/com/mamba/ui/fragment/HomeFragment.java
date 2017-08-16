package com.mamba.ui.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.framework.base.BaseFragment;
import com.framework.widgets.TopLineTabLayout;
import com.mamba.R;
import com.mamba.ui.adapter.CommonFragmentStatePagerAdapter;
import com.mamba.ui.entity.FragmentPage;

import java.util.ArrayList;
import java.util.List;

/**
 * 首页
 *
 * @author jake
 * @since 2017/8/14 下午6:19
 */

public class HomeFragment extends BaseFragment {
    private TopLineTabLayout mTabLayout;
    private ViewPager mVpContent;
    private CommonFragmentStatePagerAdapter mAdapter;

    private HomeFragment() {
    }

    public static HomeFragment newInstance() {
        return new HomeFragment();
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.fragment_home, null);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        mTabLayout = (TopLineTabLayout) findViewById(R.id.tab);
        mTabLayout.setTopLine(20,4,10);
        mVpContent = (ViewPager) findViewById(R.id.vp_content);
        setViewListener();
        mAdapter = new CommonFragmentStatePagerAdapter(getChildFragmentManager());
        List<FragmentPage> pages = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            pages.add(new FragmentPage(new HomeVideoListFragment(), "热门" ));
        }
        mAdapter.setData(pages);
        mVpContent.setAdapter(mAdapter);
        mTabLayout.setupWithViewPager(mVpContent);
        mTabLayout.getTabAt(1).select();
    }

    private void setViewListener() {
        mTabLayout.addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
        mVpContent.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
            @Override
            public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {

            }

            @Override
            public void onPageSelected(int position) {

            }

            @Override
            public void onPageScrollStateChanged(int state) {

            }
        });
    }
}
