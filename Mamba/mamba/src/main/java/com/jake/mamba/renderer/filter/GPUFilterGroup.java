package com.jake.mamba.renderer.filter;

import java.util.LinkedList;
import java.util.List;

/**
 * 多个滤镜一起使用
 *
 * @author jake
 * @since 2018/4/15 下午5:52
 */

public class GPUFilterGroup extends GPUFilter {
    private LinkedList<IGPUFilter> mFilters = new LinkedList<>();

    public void addFilter(final IGPUFilter filter) {
        if (filter != null) {
            runOnDraw(new Runnable() {
                @Override
                public void run() {
                    if (isPrepared()) {
                        filter.prepare();
                        filter.setInputTextureId(mInputTextureId);
                        filter.setViewport(mWidth, mHeight);
                    }
                    mFilters.add(filter);
                }
            });

        }
    }

    public void removeFilter(final IGPUFilter filter) {
        if (filter != null) {
            runOnDraw(new Runnable() {
                @Override
                public void run() {
                    filter.release();
                    mFilters.remove(filter);
                }
            });
        }

    }

    public List<IGPUFilter> getFilters() {
        return mFilters;
    }

    public void clear() {
        mFilters.clear();
    }

    @Override
    public void setInputTextureId(final int textureId) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                for (IGPUFilter filter : mFilters) {
                    if (filter != null) {
                        filter.setInputTextureId(textureId);
                    }
                }
            }
        });
    }

    @Override
    protected boolean onPrepare() {
        for (IGPUFilter filter : mFilters) {
            if (filter != null) {
                filter.prepare();
            }
        }
        return true;
    }

    @Override
    protected void onSetViewport(int width, int height) {
        for (IGPUFilter filter : mFilters) {
            if (filter != null) {
                filter.setViewport(width, height);
            }
        }
    }

    @Override
    protected void onPrepareDraw() {

    }


    @Override
    protected void onDraw() {
        for (IGPUFilter filter : mFilters) {
            if (filter != null) {
                filter.draw();
            }
        }
    }

    @Override
    protected void onRelease() {
        for (IGPUFilter filter : mFilters) {
            if (filter != null) {
                filter.release();
            }
        }
        clear();
    }
}
