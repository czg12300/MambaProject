package com.framework.widgets;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.support.annotation.Nullable;
import android.support.design.widget.TabLayout;
import android.support.v4.view.ViewPager;
import android.util.AttributeSet;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;

/**
 * @author jake
 * @since 2017/8/15 下午4:55
 */

public class TopLineTabLayout extends TabLayout implements ViewPager.OnPageChangeListener {
    private Paint mPaint;
    private float offsetTop = 0;
    private float lineWidth = 0;

    public TopLineTabLayout(Context context) {
        this(context, null);
    }

    public TopLineTabLayout(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }


    public TopLineTabLayout(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        TypedArray a = context.obtainStyledAttributes(attrs, android.support.design.R.styleable.TabLayout,
                defStyleAttr, android.support.design.R.style.Widget_Design_TabLayout);
        int color = a.getColor(android.support.design.R.styleable.TabLayout_tabIndicatorColor, 0);
        mPaint = new Paint();
        mPaint.setColor(color);
    }

    /**
     * 设置顶部线滑块的参数，单位为dp
     *
     * @param width
     * @param height
     * @param paddingTop
     */
    public void setTopLine(float width, float height, float paddingTop) {
        mPaint.setStrokeWidth(dp(height));
        lineWidth = dp(width);
        offsetTop = dp(paddingTop);
    }

    private float dp(float dp) {
        return TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dp, getResources().getDisplayMetrics());
    }

    @Override
    public void setupWithViewPager(@Nullable ViewPager viewPager, boolean autoRefresh) {
        super.setupWithViewPager(viewPager, autoRefresh);
        viewPager.addOnPageChangeListener(this);
    }

    private int position = -1;
    private float positionOffset = 0;

    @Override
    public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        this.position = position;
        this.positionOffset = positionOffset;
        invalidate();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (position > -1) {
            ViewGroup viewGroup = (ViewGroup) getChildAt(0);
            View view = viewGroup.getChildAt(position);
            if (view != null) {
                float left = view.getLeft();
                float right = view.getRight();
                float offsetW=((right - left)-lineWidth)/2;
                float offset = (right - left) * positionOffset;
                canvas.drawLine(left + offset+offsetW, view.getTop(), right + offset-offsetW, view.getTop(), mPaint);
            }
        }
    }

    /**
     * 计算滑块需要滑动的距离
     *
     * @param position       当前选择的位置
     * @param positionOffset 滑动位置的百分百
     * @return 滑动的距离
     */
    private int calculateScrollXForTab(int position, float positionOffset) {
//        TabLayout tabLayout = mTabLayoutRef.get();
        LinearLayout mTabStrip = (LinearLayout) getChildAt(0);
        if (mTabStrip == null) return 0;
        //当前选择的View
        final View selectedChild = mTabStrip.getChildAt(position);
        //下一个View
        final View nextChild = position + 1 < mTabStrip.getChildCount()
                ? mTabStrip.getChildAt(position + 1)
                : null;
        //当前选择的View的宽度
        final int selectedWidth = selectedChild != null ? selectedChild.getWidth() : 0;
        //下一个View的宽度
        final int nextWidth = nextChild != null ? nextChild.getWidth() : 0;
        //当前选择的View的左边位置，view的方位
        final int left = selectedChild != null ? selectedChild.getLeft() : 0;
        //计算滑块需要滑动的距离,左 + ，右 - ；
        int scrollX = -(left + ((int) ((selectedWidth + nextWidth) * positionOffset * 0.5f)));
        //现在是把SliderLayout直接放在TableLayout中的所以就不许要考虑TableLayout本身的滑动
        /*if (tabLayout.getTabMode() == TabLayout.MODE_SCROLLABLE) {//当为滑动模式的时候TabLayout会有水平方向的滑动
            scrollX += tabLayout.getScrollX();//计算在TabLayout有滑动的时候，滑块相对的滑动距离
        }*/
        return scrollX;
    }

    @Override
    public void onPageSelected(int position) {

    }

    @Override
    public void onPageScrollStateChanged(int state) {

    }
}
