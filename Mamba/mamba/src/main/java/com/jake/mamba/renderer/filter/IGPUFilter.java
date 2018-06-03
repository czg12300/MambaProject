package com.jake.mamba.renderer.filter;

/**
 * 滤镜基类，主要提供生命周期
 *
 * @author jake
 * @since 2018/4/15 下午5:50
 */

public interface IGPUFilter {
    int NO_TEXTURE = -1;

    void prepare();

    void setViewport(int width, int height);

    void draw();

    void setInputTextureId(int textureId);

    void release();
}
