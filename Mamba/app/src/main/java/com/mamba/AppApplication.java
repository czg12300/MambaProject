package com.mamba;

import android.app.Application;
import android.content.Context;

import com.framework.base.IApplication;
import com.mamba.gloable.FolderManager;

/**
 * app模块的application
 *
 * @author jake
 * @since 2017/5/7 下午9:52
 */

public class AppApplication implements IApplication {
    private static AppApplication mInstance = new AppApplication();
    private Context mAppContext;

    private AppApplication() {
    }

    public static AppApplication get() {
        return mInstance;
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
        FolderManager.initSystemFolder();
    }

    @Override
    public void onLowMemory() {

    }
}
