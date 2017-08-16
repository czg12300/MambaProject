package com.framework.widgets;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.widget.TextView;

import com.mamba.framework.R;


/**
 * 描述：类似于radioButton，配合{@link com.framework.widgets.MainTabLayoutView}使用
 *
 * @author jakechen
 * @since 2016/11/30 9:58
 */

public class MainTabView extends android.support.v7.widget.AppCompatTextView {
    private boolean isChecked = false;
    private int checkedColor;
    private int normalColor;
    private ColorFilter normalColorFilter;
    private ColorFilter checkedColorFilter;

    public MainTabView(Context context) {
        this(context, null);
    }

    public MainTabView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public MainTabView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        if (attrs != null) {
            TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.MainTabView,defStyleAttr,0);
            boolean isChecked = array.getBoolean(R.styleable.MainTabView_check, false);
            int checkedColor = array.getColor(R.styleable.MainTabView_checkedColor, Color.WHITE);
            int normalColor = array.getColor(R.styleable.MainTabView_normalColor, Color.WHITE);
            setCheckedColor(checkedColor);
            setNormalColor(normalColor);
            setChecked(isChecked);
            array.recycle();
        }

    }

    private ColorFilter color2ColorFilter(int color) {
        int red = (color & 0xFF0000) / 0xFFFF;
        int green = (color & 0xFF00) / 0xFF;
        int blue = color & 0xFF;

        float[] matrix = {0, 0, 0, 0, red, 0, 0, 0, 0, green, 0, 0, 0, 0,
                blue, 0, 0, 0, 1, 0};

        return new ColorMatrixColorFilter(matrix);
    }

    @Override
    protected void drawableStateChanged() {
        super.drawableStateChanged();
        changeDrawableState();
    }

    private void changeDrawableState() {
        Drawable[] drawables = this.getCompoundDrawables();
        if (drawables == null) {
            return;
        }
        for (Drawable drawable : drawables) {
            if (drawable == null)
                continue;
            drawable.setColorFilter(isChangeState() ? checkedColorFilter : normalColorFilter);
        }
        setTextColor(isChangeState() ? checkedColor : normalColor);
    }

    private boolean isChangeState() {
        return isChecked() || isPressed();
    }

    public boolean isChecked() {
        return isChecked;
    }

    public void setChecked(boolean checked) {
        isChecked = checked;
        changeDrawableState();
    }

    public int getCheckedColor() {
        return checkedColor;
    }

    public void setCheckedColor(int checkedColor) {
        this.checkedColor = checkedColor;
        checkedColorFilter = color2ColorFilter(checkedColor);
    }

    public int getNormalColor() {
        return normalColor;
    }

    public void setNormalColor(int normalColor) {
        this.normalColor = normalColor;
        normalColorFilter = color2ColorFilter(normalColor);
    }
}
