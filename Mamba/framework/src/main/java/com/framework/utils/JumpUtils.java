package com.framework.utils;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

/**
 * 跳转函数
 *
 * @author jake
 * @since 2017/6/21 下午4:15
 */

public class JumpUtils {
    public static final String KEY_BUNDLE = "key_bundle";

    public static void startActivity(Context context, Class<?> target) {
        startActivity(context, target, null);
    }

    public static void startActivity(Context context, Class<?> target, Bundle bundle) {
        try {
            Intent it = new Intent();
            if (!(context instanceof Activity)) {
                it.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            }
            if (bundle != null) {
                it.putExtra(KEY_BUNDLE, bundle);
            }
            it.setClass(context, target);
            context.startActivity(it);
        } catch (Exception e) {
            e.printStackTrace();
        } catch (Error e) {
            e.printStackTrace();
        }
    }

    public static Bundle getIntentBundle(Intent it) {
        if (it != null) {
            return it.getBundleExtra(KEY_BUNDLE);
        }
        return null;
    }
}
