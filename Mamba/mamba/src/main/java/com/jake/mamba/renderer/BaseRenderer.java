package com.jake.mamba.renderer;

import android.opengl.GLSurfaceView;

import com.jake.mamba.renderer.filter.IGPUFilter;

import java.util.LinkedList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * 抽象渲染器，使用GLSurfaceView作为渲染view
 *
 * @author jake
 * @since 2018/4/15 下午11:44
 */

public abstract class BaseRenderer implements GLSurfaceView.Renderer {
    private IGPUFilter mFilter;
    protected int mInputWidth;
    protected int mInputHeight;
    protected int mOutputWidth;
    protected int mOutputHeight;
    private LinkedList<Runnable> mTaskOnDraws = new LinkedList<>();
    private RendererCallback mRendererCallback;

    @Override
    public void onDrawFrame(GL10 gl) {
        onDraw(gl);
        runPendingOnDrawTasks();
        if (mRendererCallback != null) {
            mRendererCallback.onDraw();
        }
    }

    @Override
    public final void onSurfaceCreated(GL10 gl, EGLConfig config) {
        onCreated(gl, config);
        if (mRendererCallback != null) {
            mRendererCallback.onCreated();
        }
    }

    @Override
    public final void onSurfaceChanged(GL10 gl, int width, int height) {
        onChanged(gl, width, height);
        if (mRendererCallback != null) {
            mRendererCallback.onChanged(width, height);
        }
    }

    protected abstract void onDraw(GL10 gl);

    protected abstract void onCreated(GL10 gl, EGLConfig config);

    protected abstract void onChanged(GL10 gl, int width, int height);

    public interface RendererCallback {
        void onCreated();

        void onChanged(int width, int height);

        void onDraw();
    }

    protected void runInDraw(Runnable runnable) {
        synchronized (mTaskOnDraws) {
            if (runnable != null) {
                mTaskOnDraws.addLast(runnable);
            }
        }
    }

    private void runPendingOnDrawTasks() {
        while (!mTaskOnDraws.isEmpty()) {
            mTaskOnDraws.removeFirst().run();
        }
    }

    protected void setInputSize(int width, int height) {
        mInputWidth = width;
        mInputHeight = height;
    }

    protected void setOutputSize(int width, int height) {
        mOutputWidth = width;
        mOutputHeight = height;
    }

    public void setFilter(final IGPUFilter filter) {
        runInDraw(new Runnable() {
            @Override
            public void run() {
                if (filter != null) {
                    filter.prepare();
                    filter.setViewport(mInputWidth, mInputHeight);
                }
                if (mFilter != null) {
                    mFilter.release();
                }
                mFilter = filter;
            }
        });
    }

    public void setRendererCallback(RendererCallback callback) {
        this.mRendererCallback = callback;
    }
}
