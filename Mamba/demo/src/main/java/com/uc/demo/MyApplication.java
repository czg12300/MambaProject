package com.uc.demo;

import android.app.Application;


public class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
//        FrameworkApplication.get().install(this);
//        FrameworkApplication.get().onCreate();
    }
}
