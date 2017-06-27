package com.framework;

import android.content.Context;
import android.support.annotation.NonNull;

import com.framework.base.IApplication;

import java.util.List;

/**
 * framework module使用的application
 *
 * @author jake
 * @since 2017/5/7 下午9:30
 */

public class FrameworkApplication implements IApplication {
    private Context mAppContext;
    private static FrameworkApplication mInstance = new FrameworkApplication();

    public static FrameworkApplication get() {
        return mInstance;
    }

    private FrameworkApplication() {
    }


    @Override
    public Context getApplicationContext() {
        return mAppContext;
    }

    @Override
    public void install(Context context) {
        mAppContext = context;
    }


    @Override
    public void onCreate() {
        System.loadLibrary("libframework");
    }

    @Override
    public void onLowMemory() {

    }


}
