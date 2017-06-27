package com.framework.widgets;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Shader;
import android.support.v7.widget.AppCompatTextView;
import android.util.AttributeSet;
import android.widget.TextView;

public class GradientTextView extends AppCompatTextView {

    private LinearGradient mLinearGradient;
    private Paint mPaint;
    private int mViewWidth = 0;
    private Rect mTextBound = new Rect();
    private int startColor;
    private int endColor;
    private boolean hasSetGradientColor = false;

    public GradientTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setGradientColor(int startColor, int endColor) {
        hasSetGradientColor = true;
        this.startColor = startColor;
        this.endColor = endColor;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (hasSetGradientColor) {
            mViewWidth = getMeasuredWidth();
            mPaint = getPaint();
            String mTipText = getText().toString();
            mPaint.getTextBounds(mTipText, 0, mTipText.length(), mTextBound);
            mLinearGradient = new LinearGradient(0, 0, mViewWidth, 0, new int[]{startColor, endColor}, null, Shader.TileMode.REPEAT);
            mPaint.setShader(mLinearGradient);
            canvas.drawText(mTipText, getMeasuredWidth() / 2 - mTextBound.width() / 2, getMeasuredHeight() / 2 + mTextBound.height() / 2, mPaint);
        } else {
            super.onDraw(canvas);
        }
    }

}