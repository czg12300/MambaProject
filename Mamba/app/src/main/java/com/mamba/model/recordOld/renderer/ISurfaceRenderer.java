package com.mamba.model.recordOld.renderer;

import android.opengl.EGLContext;

import com.mamba.model.recordOld.renderer.gpuimage.FilterType;

/**
 * surface数据渲染
 *
 * @author jake
 * @since 2017/6/23 上午10:55
 */

public interface ISurfaceRenderer {
    void onRenderer(EGLContext sharedContext, int textureId, float[] transform, long timestamp, int width, int height, FilterType type);
}
