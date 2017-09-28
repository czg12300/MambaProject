package com.framework.widgets.jakerecycleview;

import android.animation.Animator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.support.annotation.LayoutRes;
import android.support.annotation.Nullable;
import android.support.v4.view.MotionEventCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

import com.framework.widgets.irecyclerview.*;

import java.util.ArrayList;

/**
 * 可以添加header footer和上下拉刷新加载更多的JRecycleView，
 * 调用addHeaderView、addFooterView、setRefreshView、setLoadMoreView等必须在调用setAdapter之前
 *
 * @author jake
 * @since 2017/9/19 下午4:13
 */

public class JRecycleView extends RecyclerView {
    private static final String TAG = IRecyclerView.class.getSimpleName();

    private static final int STATUS_DEFAULT = 0;

    private static final int STATUS_SWIPING_TO_REFRESH = 1;

    private static final int STATUS_RELEASE_TO_REFRESH = 2;

    private static final int STATUS_REFRESHING = 3;

    private static final boolean DEBUG = false;

    private int mStatus;

    private boolean mIsAutoRefreshing;

    private boolean mRefreshEnabled = false;

    private boolean mLoadMoreEnabled = false;

    private int mRefreshFinalMoveOffset;

    private ArrayList<OnRefreshListener> mOnRefreshListeners;

    private ArrayList<OnLoadMoreListener> mOnLoadMoreListeners;

    private OnLoadMoreScrollListener mOnLoadMoreScrollListener;
    private LinearLayout mHeaderContainer;
    private LinearLayout mFooterContainer;
    private DragHelper mDragHelper;

    public JRecycleView(Context context) {
        this(context, null);
    }

    public JRecycleView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public JRecycleView(Context context, @Nullable AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mDragHelper = new DragHelper();
    }

    private LinearLayout getHeaderContainer() {
        if (mHeaderContainer == null) {
            mHeaderContainer = new LinearLayout(getContext());
            mHeaderContainer.setOrientation(isHorizontal() ? LinearLayout.HORIZONTAL : LinearLayout.VERTICAL);
        }
        return mHeaderContainer;
    }

    private LinearLayout getFooterContainer() {
        if (mFooterContainer == null) {
            mFooterContainer = new LinearLayout(getContext());
            mFooterContainer.setOrientation(isHorizontal() ? LinearLayout.HORIZONTAL : LinearLayout.VERTICAL);
        }
        return mFooterContainer;
    }

    private boolean isHorizontal() {
        if (getLayoutManager() instanceof LinearLayoutManager) {
            LinearLayoutManager manager = (LinearLayoutManager) getLayoutManager();
            return manager.getOrientation() == LinearLayoutManager.HORIZONTAL;
        }
        return false;

    }

    @Override
    protected void onMeasure(int widthSpec, int heightSpec) {
        super.onMeasure(widthSpec, heightSpec);
        if (mRefreshHeaderView != null) {
            if (mRefreshHeaderView.getMeasuredHeight() > mRefreshFinalMoveOffset) {
                mRefreshFinalMoveOffset = 0;
            }
        }
    }

    public void setRefreshEnabled(boolean enabled) {
        this.mRefreshEnabled = enabled;
    }

    public boolean refreshEnabled() {
        return mRefreshEnabled;
    }

    public boolean loadMoreEnabled() {
        return mLoadMoreEnabled;
    }

    public void setLoadMoreEnabled(boolean enabled) {
        this.mLoadMoreEnabled = enabled;
    }


    public void addOnRefreshListener(OnRefreshListener listener) {
        if (listener != null) {
            if (mOnRefreshListeners == null) {
                mOnRefreshListeners = new ArrayList<>();
            }
            mOnRefreshListeners.add(listener);
        }
    }

    public void addOnLoadMoreListener(OnLoadMoreListener listener) {
        if (listener != null) {
            if (mOnLoadMoreListeners == null) {
                mOnLoadMoreListeners = new ArrayList<>();
            }
            mOnLoadMoreListeners.add(listener);
        }
    }

    public void setRefreshing(boolean refreshing) {
        if (mStatus == STATUS_DEFAULT && refreshing) {
            this.mIsAutoRefreshing = true;
            setStatus(STATUS_SWIPING_TO_REFRESH);
            startScrollDefaultStatusToRefreshingStatus();
        } else if (mStatus == STATUS_REFRESHING && !refreshing) {
            this.mIsAutoRefreshing = false;
            startScrollRefreshingStatusToDefaultStatus();
        } else {
            this.mIsAutoRefreshing = false;
            Log.e(TAG, "isRefresh = " + refreshing + " current status = " + mStatus);
        }
    }

    public void setRefreshFinalMoveOffset(int refreshFinalMoveOffset) {
        this.mRefreshFinalMoveOffset = refreshFinalMoveOffset;
    }

    public void setRefreshView(View refreshHeaderView) {
        if (!(refreshHeaderView instanceof DragTrigger)) {
            throw new ClassCastException("Refresh header view must be an implement of DragTrigger");
        }
        mDragHelper.setRefreshDragTrigger((DragTrigger) refreshHeaderView);
        if (getHeaderContainer().getChildCount() > 0) {
            View child0 = getHeaderContainer().getChildAt(0);
            if (child0 instanceof DragTrigger) {
                getHeaderContainer().removeView(child0);
            }
        }
        getHeaderContainer().addView(refreshHeaderView, 0);
    }

    public void setRefreshView(@LayoutRes int refreshHeaderLayoutRes) {
        setRefreshView(inflate(getContext(), refreshHeaderLayoutRes, null));
    }

    public void setLoadMoreView(View view) {
        if (!(view instanceof DragTrigger)) {
            throw new ClassCastException("Refresh header view must be an implement of DragTrigger");
        }
        mDragHelper.setRefreshDragTrigger((DragTrigger) view);
        getHeaderContainer().addView(view);
    }

    public void setLoadMoreView(@LayoutRes int layoutId) {
        setLoadMoreView(inflate(getContext(), layoutId, null));
    }

    public View getRefreshHeaderView() {
        return mRefreshHeaderView;
    }

    public View getLoadMoreFooterView() {
        return mLoadMoreFooterView;
    }

//
//    public LinearLayout getFooterContainer() {
//        ensureFooterViewContainer();
//        return mFooterViewContainer;
//    }

    public void addHeaderView(View headerView) {
        getHeaderContainer().addView(headerView);
        Adapter adapter = getAdapter();
        if (adapter != null) {
            adapter.notifyItemChanged(0);
        }
    }

    public void addFooterView(View footerView) {
        if (getFooterContainer().getChildCount() > 0) {
            View loadMoreView = getFooterContainer().getChildAt(getFooterContainer().getChildCount() - 1);
            if (loadMoreView instanceof DragTrigger) {
                getFooterContainer().addView(footerView, getFooterContainer().getChildCount() - 2);
            } else {
                getFooterContainer().addView(footerView);
            }
        }
        Adapter adapter = getAdapter();
        if (adapter != null) {
            adapter.notifyItemChanged(adapter.getItemCount() - 1);
        }
    }

    @Override
    public Adapter getAdapter() {
        Adapter adapter = super.getAdapter();
        if (adapter instanceof WrapperAdapter) {
            adapter = ((WrapperAdapter) adapter).getAdapter();
        }
        return adapter;
    }

    @Override
    public void setAdapter(Adapter adapter) {
        super.setAdapter(adapter);

    }
//
//    public void setIAdapter(Adapter adapter) {
//        ensureRefreshHeaderContainer();
//        ensureHeaderViewContainer();
//        ensureFooterViewContainer();
//        ensureLoadMoreFooterContainer();
//        setAdapter(new com.framework.widgets.irecyclerview.WrapperAdapter(adapter, mRefreshHeaderContainer, mHeaderViewContainer, mFooterViewContainer, mLoadMoreFooterContainer));
//    }

    private boolean isRefreshTrigger(View refreshHeaderView) {
        return refreshHeaderView instanceof RefreshTrigger;
    }

    private void removeRefreshHeaderView() {
        if (mRefreshHeaderContainer != null) {
            mRefreshHeaderContainer.removeView(mRefreshHeaderView);
        }
    }

    private void removeLoadMoreFooterView() {
        if (mLoadMoreFooterContainer != null) {
            mLoadMoreFooterContainer.removeView(mLoadMoreFooterView);
        }
    }


    private boolean isFingerDragging() {
        return getScrollState() == SCROLL_STATE_DRAGGING;
    }

    public boolean canTriggerRefresh() {
        final Adapter adapter = getAdapter();
        if (adapter == null || adapter.getItemCount() <= 0) {
            return true;
        }
        View firstChild = getChildAt(0);
        int position = getChildLayoutPosition(firstChild);
        if (position == 0) {
            if (firstChild.getTop() == mRefreshHeaderContainer.getTop()) {
                return true;
            }
        }
        return false;
    }

    private int getMotionEventX(MotionEvent e, int pointerIndex) {
        return (int) (MotionEventCompat.getX(e, pointerIndex) + 0.5f);
    }

    private int getMotionEventY(MotionEvent e, int pointerIndex) {
        return (int) (MotionEventCompat.getY(e, pointerIndex) + 0.5f);
    }

    private void fingerMove(int dy) {
        int ratioDy = (int) (dy * 0.5f + 0.5);
        int offset = mRefreshHeaderContainer.getMeasuredHeight();
        int finalDragOffset = mRefreshFinalMoveOffset;

        int nextOffset = offset + ratioDy;
        if (finalDragOffset > 0) {
            if (nextOffset > finalDragOffset) {
                ratioDy = finalDragOffset - offset;
            }
        }

        if (nextOffset < 0) {
            ratioDy = -offset;
        }
        move(ratioDy);
    }

    private void move(int dy) {
        if (dy != 0) {
            int height = mRefreshHeaderContainer.getMeasuredHeight() + dy;
            setRefreshHeaderContainerHeight(height);
            mRefreshTrigger.onMove(false, false, height);
        }
    }

    private void setRefreshHeaderContainerHeight(int height) {
        mRefreshHeaderContainer.getLayoutParams().height = height;
        mRefreshHeaderContainer.requestLayout();
    }

    private void startScrollDefaultStatusToRefreshingStatus() {
        mRefreshTrigger.onStart(true, mRefreshHeaderView.getMeasuredHeight(), mRefreshFinalMoveOffset);

        int targetHeight = mRefreshHeaderView.getMeasuredHeight();
        int currentHeight = mRefreshHeaderContainer.getMeasuredHeight();
        startScrollAnimation(400, new AccelerateInterpolator(), currentHeight, targetHeight);
    }

    private void startScrollSwipingToRefreshStatusToDefaultStatus() {
        final int targetHeight = 0;
        final int currentHeight = mRefreshHeaderContainer.getMeasuredHeight();
        startScrollAnimation(300, new DecelerateInterpolator(), currentHeight, targetHeight);
    }

    private void startScrollReleaseStatusToRefreshingStatus() {
        mRefreshTrigger.onRelease();

        final int targetHeight = mRefreshHeaderView.getMeasuredHeight();
        final int currentHeight = mRefreshHeaderContainer.getMeasuredHeight();
        startScrollAnimation(300, new DecelerateInterpolator(), currentHeight, targetHeight);
    }

    private void startScrollRefreshingStatusToDefaultStatus() {
        mRefreshTrigger.onComplete();

        final int targetHeight = 0;
        final int currentHeight = mRefreshHeaderContainer.getMeasuredHeight();
        startScrollAnimation(400, new DecelerateInterpolator(), currentHeight, targetHeight);
    }

    ValueAnimator mScrollAnimator;

    private void startScrollAnimation(final int time, final Interpolator interpolator, int value, int toValue) {
        if (mScrollAnimator == null) {
            mScrollAnimator = new ValueAnimator();
        }
        //cancel
        mScrollAnimator.removeAllUpdateListeners();
        mScrollAnimator.removeAllListeners();
        mScrollAnimator.cancel();

        //reset new value
        mScrollAnimator.setIntValues(value, toValue);
        mScrollAnimator.setDuration(time);
        mScrollAnimator.setInterpolator(interpolator);
        mScrollAnimator.addUpdateListener(mAnimatorUpdateListener);
        mScrollAnimator.addListener(mAnimationListener);
        mScrollAnimator.start();
    }

    ValueAnimator.AnimatorUpdateListener mAnimatorUpdateListener = new ValueAnimator.AnimatorUpdateListener() {
        @Override
        public void onAnimationUpdate(ValueAnimator animation) {
            final int height = (Integer) animation.getAnimatedValue();
            setRefreshHeaderContainerHeight(height);
            switch (mStatus) {
                case STATUS_SWIPING_TO_REFRESH: {
                    mRefreshTrigger.onMove(false, true, height);
                }
                break;

                case STATUS_RELEASE_TO_REFRESH: {
                    mRefreshTrigger.onMove(false, true, height);
                }
                break;

                case STATUS_REFRESHING: {
                    mRefreshTrigger.onMove(true, true, height);
                }
                break;
            }

        }
    };

    Animator.AnimatorListener mAnimationListener = new SimpleAnimatorListener() {
        @Override
        public void onAnimationEnd(Animator animation) {
            int lastStatus = mStatus;

            switch (mStatus) {
                case STATUS_SWIPING_TO_REFRESH: {
                    if (mIsAutoRefreshing) {
                        mRefreshHeaderContainer.getLayoutParams().height = mRefreshHeaderView.getMeasuredHeight();
                        mRefreshHeaderContainer.requestLayout();
                        setStatus(STATUS_REFRESHING);
                        if (mOnRefreshListener != null) {
//                            mOnRefreshListener.onRefresh();
                            mRefreshTrigger.onRefresh();
                        }
                    } else {
                        mRefreshHeaderContainer.getLayoutParams().height = 0;
                        mRefreshHeaderContainer.requestLayout();
                        setStatus(STATUS_DEFAULT);
                    }
                }
                break;

                case STATUS_RELEASE_TO_REFRESH: {
                    mRefreshHeaderContainer.getLayoutParams().height = mRefreshHeaderView.getMeasuredHeight();
                    mRefreshHeaderContainer.requestLayout();
                    setStatus(STATUS_REFRESHING);
                    if (mOnRefreshListener != null) {
//                        mOnRefreshListener.onRefresh();
                        mRefreshTrigger.onRefresh();
                    }
                }
                break;

                case STATUS_REFRESHING: {
                    mIsAutoRefreshing = false;
                    mRefreshHeaderContainer.getLayoutParams().height = 0;
                    mRefreshHeaderContainer.requestLayout();
                    setStatus(STATUS_DEFAULT);
                    mRefreshTrigger.onReset();
                }
                break;
            }
            if (DEBUG) {
                Log.i(TAG, "onAnimationEnd " + getStatusLog(lastStatus) + " -> " + getStatusLog(mStatus) + " ;refresh view height:" + mRefreshHeaderContainer.getMeasuredHeight());
            }
        }
    };

    private void onFingerUpStartAnimating() {
        if (mStatus == STATUS_RELEASE_TO_REFRESH) {
            startScrollReleaseStatusToRefreshingStatus();
        } else if (mStatus == STATUS_SWIPING_TO_REFRESH) {
            startScrollSwipingToRefreshStatusToDefaultStatus();
        }
    }

    private void onPointerUp(MotionEvent e) {
        final int actionIndex = MotionEventCompat.getActionIndex(e);
        if (MotionEventCompat.getPointerId(e, actionIndex) == mActivePointerId) {
            // Pick a new pointer to pick up the slack.
            final int newIndex = actionIndex == 0 ? 1 : 0;
            mActivePointerId = MotionEventCompat.getPointerId(e, newIndex);
            mLastTouchX = getMotionEventX(e, newIndex);
            mLastTouchY = getMotionEventY(e, newIndex);
        }
    }

    RefreshTrigger mRefreshTrigger = new RefreshTrigger() {
        @Override
        public void onStart(boolean automatic, int headerHeight, int finalHeight) {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onStart(automatic, headerHeight, finalHeight);
            }
        }

        @Override
        public void onMove(boolean finished, boolean automatic, int moved) {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onMove(finished, automatic, moved);
            }
        }

        @Override
        public void onRefresh() {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onRefresh();
            }
        }

        @Override
        public void onRelease() {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onRelease();
            }
        }

        @Override
        public void onComplete() {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onComplete();
            }
        }

        @Override
        public void onReset() {
            if (mRefreshHeaderView != null && mRefreshHeaderView instanceof RefreshTrigger) {
                RefreshTrigger trigger = (RefreshTrigger) mRefreshHeaderView;
                trigger.onReset();
            }
        }
    };

    private void setStatus(int status) {
        this.mStatus = status;
        if (DEBUG) {
            printStatusLog();
        }
    }

    private void printStatusLog() {
        Log.i(TAG, getStatusLog(mStatus));
    }

    private String getStatusLog(int status) {
        final String statusLog;
        switch (status) {
            case STATUS_DEFAULT:
                statusLog = "status_default";
                break;

            case STATUS_SWIPING_TO_REFRESH:
                statusLog = "status_swiping_to_refresh";
                break;

            case STATUS_RELEASE_TO_REFRESH:
                statusLog = "status_release_to_refresh";
                break;

            case STATUS_REFRESHING:
                statusLog = "status_refreshing";
                break;
            default:
                statusLog = "status_illegal!";
                break;
        }
        return statusLog;
    }

    public abstract static class ViewHolder extends RecyclerView.ViewHolder {

        public ViewHolder(View itemView) {
            super(itemView);
        }

        @Deprecated
        public final int getIPosition() {
            return getPosition() - 1;
        }

        public final int getILayoutPosition() {
            return getLayoutPosition() - 1;
        }

        public final int getIAdapterPosition() {
            return getAdapterPosition() - 1;
        }

        public final int getIOldPosition() {
            return getOldPosition() - 1;
        }

        public final long getIItemId() {
            return getItemId();
        }

        public final int getIItemViewType() {
            return getItemViewType();
        }
    }
}
