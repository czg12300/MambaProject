package com.mamba;

import android.graphics.Color;
import android.os.CountDownTimer;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import com.framework.base.BaseActivity;
import com.framework.utils.JumpUtils;
import com.framework.widgets.GradientTextView;
import com.mamba.ui.RecordActivity;
import com.mamba.ui.activity.MainActivity;

public class SplashActivity extends BaseActivity {
    private GradientTextView mTvSlogan;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);
        mTvSlogan = (GradientTextView) findViewById(R.id.tv_slogan);
        mTvSlogan.setText("Mamba");
        mTvSlogan.setGradientColor(Color.parseColor("#8E049E"), Color.parseColor("#FF4640"));
        new CountDownTimer(1000, 1000) {
            @Override
            public void onTick(long millisUntilFinished) {

            }

            @Override
            public void onFinish() {
                JumpUtils.startActivity(mTvSlogan.getContext(), MainActivity.class);
                finish();
            }
        }.start();
    }
}
