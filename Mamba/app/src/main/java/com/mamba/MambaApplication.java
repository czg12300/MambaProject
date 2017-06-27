package com.mamba;

import com.framework.FrameworkApplication;
import com.framework.base.IApplication;

import java.util.ArrayList;

/**
 * 全局变量
 *
 * @author jake
 * @since 2017/5/7 下午9:16
 */

public class MambaApplication extends android.app.Application {
    private ArrayList<IApplication> mApplicationList = new ArrayList<>();


    @Override
    public void onCreate() {
        super.onCreate();
        registerApplication(FrameworkApplication.get());
        registerApplication(AppApplication.get());
        for (IApplication application : mApplicationList) {
            application.onCreate();
        }
    }

    /**
     * 注册application
     *
     * @param application
     */
    private void registerApplication(IApplication application) {
        application.install(getApplicationContext());
        mApplicationList.add(application);
    }


    @Override
    public void onLowMemory() {
        super.onLowMemory();
        for (IApplication application : mApplicationList) {
            application.onLowMemory();
        }
    }

}
