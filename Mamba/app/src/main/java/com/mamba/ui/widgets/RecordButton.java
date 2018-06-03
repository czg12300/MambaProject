package com.mamba.ui.widgets;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.drawable.Drawable;
import android.support.annotation.AttrRes;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;


public class RecordButton extends FrameLayout implements View.OnTouchListener {


    public enum Mode {
        TOUCH, CLICK
    }

    private ImageView mCameraImageView;
    private ValueAnimator mStartAnimator;
    private ValueAnimator mEndAnimator;
    private ValueAnimator mRepeatAnimator;
    private Paint mPaint;
    private Callback mCallback = null;
    private int mRawX;
    private int mRawY;
    private float mMinRingWidth;
    private float mInsideCircleRadius;
    private Mode mMode = Mode.TOUCH;
    private boolean mIsRecording = false;

    public RecordButton(@NonNull Context context) {
        this(context, null);
    }

    public RecordButton(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public RecordButton(@NonNull Context context, @Nullable AttributeSet attrs, @AttrRes int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        mCameraImageView = new ImageView(getContext());
        mCameraImageView.setScaleType(ImageView.ScaleType.CENTER_INSIDE);
        LayoutParams lp = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.CENTER;
        addView(mCameraImageView, lp);
        mMinRingWidth = dip2px(4f);
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setColor(0xfffe2c55);
        mPaint.setStyle(Paint.Style.FILL);

        setOnTouchListener(this);
        setWillNotDraw(false);
    }

    public void setMode(Mode mode) {
        mMode = mode;
    }

    public void setCircleColor(int color) {
        mPaint.setColor(color);
    }

    public void setMinRingWidth(float px) {
        mMinRingWidth = px;
    }

    public void setIconResource(int resId) {
        mCameraImageView.setImageResource(resId);
    }

    public void setIconImageDrawable(Drawable drawable) {
        mCameraImageView.setImageDrawable(drawable);
    }

    public void setIconImageBitmap(Bitmap bitmap) {
        mCameraImageView.setImageBitmap(bitmap);
    }

    public ImageView getIconImageView() {
        return mCameraImageView;
    }

    private int dip2px(float value) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, value, getResources().getDisplayMetrics());
    }

    public Mode getMode() {
        return mMode;
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (!mMode.equals(Mode.TOUCH)) {
            return false;
        }
        int action = event.getAction();
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                mRawX = (int) event.getRawX();
                mRawY = (int) event.getRawY();
                if (mCallback != null && mCallback.preventRecord()) {
                    return false;
                }
                startRecord();
                return true;
            case MotionEvent.ACTION_MOVE:
                animate().translationX((float) (((int) event.getRawX()) - mRawX)).translationY((float) (((int) event.getRawY()) - mRawY)).setDuration(0).start();
                return false;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                endRecord();
                return false;
            default:
                return false;
        }
    }

    public boolean isRecording() {
        return mIsRecording;
    }


    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        float halfWidth = getWidth() / 2.0f;
        mPaint.setXfermode(null);
        canvas.drawCircle(halfWidth, halfWidth, halfWidth, mPaint);
        mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.CLEAR));
        canvas.drawCircle(halfWidth, halfWidth, mInsideCircleRadius, mPaint);
    }


    public void startRecord() {
        mIsRecording = true;
        if (mCallback != null) {
            mCallback.onStartRecord();
        }
        mCameraImageView.animate().scaleX(0.0f).scaleY(0.0f).alpha(0.0f).setDuration(100).start();
        if (mStartAnimator != null) {
            mStartAnimator.cancel();
        }
        mStartAnimator = ValueAnimator.ofFloat(1.0f, 1.5f);
        mStartAnimator.setDuration(300);
        mStartAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                float value = (float) animation.getAnimatedValue();
                setScaleX(value);
                setScaleY(value);
                float present = (value - 1) * 2;
                mInsideCircleRadius = (getWidth() / 2.0f - mMinRingWidth) * present;
                invalidate();
            }
        });
        mStartAnimator.addListener(new AnimatorListenerAdapter() {
            @Override
            public void onAnimationEnd(Animator animation) {
                super.onAnimationEnd(animation);
                startRepeatAnimation();
            }
        });
        mStartAnimator.start();
    }

    private void startRepeatAnimation() {
        mRepeatAnimator = ValueAnimator.ofFloat(1f, 0.85f);
        mRepeatAnimator.setRepeatMode(ValueAnimator.REVERSE);
        mRepeatAnimator.setDuration(500);
        mRepeatAnimator.setRepeatCount(100000);
        mRepeatAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                float value = (float) animation.getAnimatedValue();
                mInsideCircleRadius = (getWidth() / 2.0f - mMinRingWidth) * value;
                invalidate();
            }
        });
        mRepeatAnimator.start();
    }

    public void endRecord() {
        mIsRecording = false;
        if (this.mCallback != null) {
            mCallback.onEndRecord();
        }
        if (mRepeatAnimator != null) {
            mRepeatAnimator.cancel();
        }
        if (mEndAnimator != null) {
            mEndAnimator.cancel();
        }
        mEndAnimator = ValueAnimator.ofFloat(1.50f, 1.0f);
        mEndAnimator.setDuration(300);
        mEndAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                float value = (float) animation.getAnimatedValue();
                setScaleX(value);
                setScaleY(value);
                float present = (value - 1) * 2;
                mInsideCircleRadius = (getWidth() / 2.0f - mMinRingWidth) * present;
                invalidate();
            }
        });
        mEndAnimator.start();
        animate().setDuration(300).translationX(0.0f).translationY(0.0f).start();
        mCameraImageView.animate().scaleX(1.0f).scaleY(1.0f).alpha(1.0f).setDuration(300).start();
    }


    public void setCallback(Callback callback) {
        this.mCallback = callback;
    }

    public interface Callback {
        void onStartRecord();

        void onEndRecord();

        boolean preventRecord();
    }
}