package com.framework.base;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.CallSuper;
import android.view.Window;
import android.view.WindowManager;

/**
 * dialog基类
 *
 * @author jake
 * @since 2017/7/18 下午2:14
 */

public class BaseDialog extends Dialog {
    public BaseDialog(Context context) {
        super(context);
        configWindow(context, getWindow());
    }

    public BaseDialog(Context context, int themeResId) {
        super(context, themeResId);
        configWindow(context, getWindow());
    }

    /**
     * 是否由后台创建
     *
     * @param context
     * @return
     */
    private boolean isCreatedByBack(Context context) {
        return !(context instanceof Activity);
    }

    /**
     * 配置window的样式,子类重写一定要调用super方法
     *
     * @param context
     * @param window
     */
    @CallSuper
    protected void configWindow(Context context, Window window) {
        if (isCreatedByBack(context)) {
            this.getWindow().setType((WindowManager.LayoutParams.TYPE_TOAST));
        }
    }

    /**
     * 获取string资源文件
     *
     * @param stringId
     * @return
     */
    protected String getString(int stringId) {
        return getContext() != null ? getContext().getString(stringId) : null;
    }

    /**
     * 获取color资源文件
     *
     * @param colorId
     * @return
     */
    protected int getColor(int colorId) {
        return getContext() != null ? getContext().getResources().getColor(colorId)
                : Color.TRANSPARENT;
    }
}
