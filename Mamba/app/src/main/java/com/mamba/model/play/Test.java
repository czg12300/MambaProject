package com.mamba.model.play;

import android.view.Surface;

/**
 * @author jake
 * @since 2017/7/26 上午10:24
 */

public interface Test {
    void toggle();

    void setSurface(Surface surface);

    void start(String filePath);

    void setListener(Listener listener);

    interface Listener {
        void onStart();

        void onFinish();
    }
}
