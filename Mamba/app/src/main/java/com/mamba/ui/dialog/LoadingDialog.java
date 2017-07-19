package com.mamba.ui.dialog;

import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ProgressBar;

import com.framework.base.BaseDialog;
import com.mamba.R;

/**
 * 加载框动画
 *
 * @author jake
 * @since 2017/7/18 下午2:13
 */

public class LoadingDialog extends BaseDialog {
    private ProgressBar progressBar;

    public LoadingDialog(Context context) {
        super(context, R.style.PopDialogTheme);
        View view = View.inflate(context, R.layout.dialog_loading_no_progress, null);
        setContentView(view);
//        setContentView(R.layout.dialog_loading_no_progress);
        progressBar = (ProgressBar) findViewById(R.id.progressBar);
    }


    @Override
    protected void configWindow(Context context, Window window) {
        super.configWindow(context, window);
        window.setGravity(Gravity.CENTER);
//        int horizontalPadding = context.getResources().getDimensionPixelSize(R.dimen.px_100);
//        int verticalPadding = context.getResources().getDimensionPixelSize(R.dimen.px_100);
//        window.getDecorView().setPadding(horizontalPadding, verticalPadding, horizontalPadding,
//                verticalPadding);
        WindowManager.LayoutParams lp = window.getAttributes();
        lp.width = WindowManager.LayoutParams.WRAP_CONTENT;
        lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
        window.setAttributes(lp);
    }
}
