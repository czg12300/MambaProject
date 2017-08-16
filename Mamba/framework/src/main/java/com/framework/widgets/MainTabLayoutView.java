package com.framework.widgets;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

import java.util.ArrayList;

/**
 * 描述：类似于radioGroup的功能
 *
 * @author jakechen
 * @since 2016/11/30 10:00
 */

public class MainTabLayoutView extends LinearLayout implements View.OnClickListener {
    private ArrayList<MainTabView> mTabViews = new ArrayList<>();
    private OnTabCheckedChangeListener mOnTabCheckedChangeListener;

    public MainTabLayoutView(Context context) {
        super(context);
        init();
    }


    public MainTabLayoutView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public MainTabLayoutView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        setFocusable(true);
        setFocusableInTouchMode(true);
        setOnHierarchyChangeListener(new OnHierarchyChangeListener() {
            @Override
            public void onChildViewAdded(View parent, View child) {
                if (parent == MainTabLayoutView.this && child instanceof MainTabView) {
                    int id = child.getId();
                    // generates an id if it's missing
                    if (id == View.NO_ID) {
                        id = child.hashCode();
                        child.setId(id);
                    }
                    addTabView((MainTabView) child);
                } else if (parent == MainTabLayoutView.this && child instanceof ViewGroup) {
                    MainTabView btn = findMainTabView((ViewGroup) child);
                    if (btn != null) {
                        int id = btn.getId();
                        // generates an id if it's missing
                        if (id == View.NO_ID) {
                            id = btn.hashCode();
                            btn.setId(id);
                        }
                        addTabView(btn);
                    }
                }

            }

            @Override
            public void onChildViewRemoved(View parent, View child) {
                if (parent == MainTabLayoutView.this && child instanceof MainTabView) {
                    mTabViews.remove(child);
                } else if (parent == MainTabLayoutView.this && child instanceof ViewGroup) {
                    mTabViews.remove(findMainTabView((ViewGroup) child));
                }
            }
        });
    }

    private void addTabView(MainTabView tabView) {
        if (!mTabViews.contains(tabView)) {
            tabView.setOnClickListener(this);
            mTabViews.add(tabView);
        }
    }

    public void check(int tabViewId) {
        if (mTabViews != null) {
            int size = mTabViews.size();
            for (int i = 0; i < size; i++) {
                MainTabView tabView = mTabViews.get(i);
                if (tabView != null) {
                    if (tabViewId == tabView.getId()) {
                        tabView.setChecked(true);
                    } else {
                        tabView.setChecked(false);
                    }
                }
            }
        }
    }

    public void check(MainTabView tabView) {
        if (tabView == null) {
            return;
        }
        int id = tabView.getId();
        check(id);

    }

    /**
     * 查找MainTabView控件
     */
    public MainTabView findMainTabView(ViewGroup group) {
        MainTabView resBtn = null;
        int len = group.getChildCount();
        for (int i = 0; i < len; i++) {
            if (group.getChildAt(i) instanceof MainTabView) {
                resBtn = (MainTabView) group.getChildAt(i);
            } else if (group.getChildAt(i) instanceof ViewGroup) {
                findMainTabView((ViewGroup) group.getChildAt(i));
            }
        }
        return resBtn;
    }

    @Override
    public void onClick(View v) {
        if (mOnTabCheckedChangeListener != null) {
            mOnTabCheckedChangeListener.onCheck(v.getId());
        }
    }

    public void setOnTabCheckChangeListener(OnTabCheckedChangeListener listener) {
        mOnTabCheckedChangeListener = listener;
    }

    public static interface OnTabCheckedChangeListener {
        void onCheck(int checkedId);
    }
}