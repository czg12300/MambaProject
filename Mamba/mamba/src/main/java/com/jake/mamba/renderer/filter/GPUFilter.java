package com.jake.mamba.renderer.filter;

import java.util.LinkedList;

/**
 * 实现生命周期回调
 *
 * @author jake
 * @since 2018/4/15 下午5:52
 */

public abstract class GPUFilter implements IGPUFilter {
    private boolean mIsPrepared = false;
    protected int mWidth;
    protected int mHeight;
    protected int mInputTextureId = NO_TEXTURE;
    private LinkedList<Runnable> mTaskOnDraws = new LinkedList<>();

    @Override
    public void prepare() {
        mIsPrepared = onPrepare();
    }

    protected abstract boolean onPrepare();

    @Override
    public void setViewport(int width, int height) {
        mWidth = width;
        mHeight = height;
        onSetViewport(width, height);
    }


    protected void onSetViewport(int width, int height) {
    }

    @Override
    public void setInputTextureId(int textureId) {
        mInputTextureId = textureId;
    }

    @Override
    public void draw() {
        if (!mIsPrepared && needInterruptOnDraw()) {
            return;
        }
        onPrepareDraw();
        runPendingOnDrawTasks();
        onDraw();
    }

    protected abstract void onPrepareDraw();

    private void runPendingOnDrawTasks() {
        while (!mTaskOnDraws.isEmpty()) {
            mTaskOnDraws.removeFirst().run();
        }
    }

    protected void runOnDraw(Runnable runnable) {
        synchronized (mTaskOnDraws) {
            if (runnable != null) {
                mTaskOnDraws.addLast(runnable);
            }
        }
    }

    protected boolean needInterruptOnDraw() {
        return false;
    }

    protected abstract void onDraw();

    @Override
    public void release() {
        mIsPrepared = false;
        onRelease();
    }

    protected abstract void onRelease();

    protected boolean isPrepared() {
        return mIsPrepared;
    }
}
