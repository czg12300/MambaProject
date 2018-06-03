package com.jake.mamba.utils;

import android.util.Log;

/**
 * @author jake
 * @since 2018/6/3 下午1:21
 */

public class L {
    static boolean DEBUG = false;
    static String TAG = "jake";

    public static void d(String msg) {
      d(TAG,msg);
    }
    public static void d(String tag,String msg) {
        Log.d(tag, msg);
    }
}
