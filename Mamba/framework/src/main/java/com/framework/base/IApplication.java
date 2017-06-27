package com.framework.base;

import android.content.Context;
import android.support.annotation.NonNull;

import java.util.List;

/**
 * 所有application的基类
 *
 * @author jake
 * @since 2017/5/7 下午9:17
 */

public interface IApplication {
    Context getApplicationContext();

    /**
     * 此方法会在application创建的时候调用
     *
     * @param context
     */
    void install(Context context);

    void onCreate();

    void onLowMemory();
}
