package com.framework.widgets.jakerecycleview;

/**
 * 拖动状态回调监听
 *
 * @author jake
 * @since 2017/9/21 上午11:59
 */

public interface DragTrigger {
    void onStart(boolean automatic, int headerHeight, int finalHeight);

    void onMove(boolean finished, boolean automatic, int moved);

    void onRefresh();

    void onRelease();

    void onComplete();

    void onReset();
}
