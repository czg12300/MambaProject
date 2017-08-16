package com.mamba.ui.activity;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;

import com.framework.base.BaseActivity;
import com.framework.utils.JumpUtils;
import com.framework.widgets.MainTabLayoutView;
import com.framework.widgets.MainTabViewPager;
import com.mamba.R;
import com.mamba.ui.RecordActivity;
import com.mamba.ui.adapter.CommonFragmentPagerAdapter;
import com.mamba.ui.entity.FragmentPage;
import com.mamba.ui.fragment.HomeFragment;
import com.mamba.ui.fragment.MessageFragment;
import com.mamba.ui.fragment.MineFragment;
import com.mamba.ui.fragment.TopicFragment;

import java.util.ArrayList;
import java.util.List;

/**
 * 主页
 *
 * @author jake
 * @since 2017/8/14 下午4:06
 */

public class MainActivity extends BaseActivity {
    private MainTabLayoutView mTab;
    private MainTabViewPager mVpContent;
    private CommonFragmentPagerAdapter mAdapter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViews();
        setListeners();
        initData();
    }

    private void findViews() {
        mTab = (MainTabLayoutView) findViewById(R.id.tab);
        mVpContent = (MainTabViewPager) findViewById(R.id.vp_content);
    }

    private void setListeners() {
        findViewById(R.id.btn_camera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                JumpUtils.startActivity(v.getContext(), RecordActivity.class);
            }
        });
        mTab.setOnTabCheckChangeListener(new MainTabLayoutView.OnTabCheckedChangeListener() {
            @Override
            public void onCheck(int checkedId) {
                switch (checkedId) {
                    case R.id.btn_home:
                        mVpContent.setCurrentItem(0);
                        break;
                    case R.id.btn_topic:
                        mVpContent.setCurrentItem(1);
                        break;
                    case R.id.btn_msg:
                        mVpContent.setCurrentItem(2);
                        break;
                    case R.id.btn_mine:
                        mVpContent.setCurrentItem(3);
                        break;
                }
            }
        });
    }

    private void initData() {
        mAdapter = new CommonFragmentPagerAdapter(getSupportFragmentManager());
        List<FragmentPage> pages = new ArrayList<>();
        pages.add(new FragmentPage(HomeFragment.newInstance()));
        pages.add(new FragmentPage(TopicFragment.newInstance()));
        pages.add(new FragmentPage(MessageFragment.newInstance()));
        pages.add(new FragmentPage(MineFragment.newInstance()));
        mAdapter.setData(pages);
        mVpContent.setCanScroll(false);
        mVpContent.setAdapter(mAdapter);
    }


}
