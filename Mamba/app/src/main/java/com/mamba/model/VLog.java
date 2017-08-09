package com.mamba.model;

import android.util.Log;

/**
 * 日志工具
 *
 * @author jake
 */
public class VLog {

    private static final String TAG = "mamba_log";

    private static boolean isDebug = true;

    /**
     * 是否处于调试模式
     */
    public static boolean isDebug() {
        return isDebug;
    }

    /**
     * 设置是否调试模式
     *
     * @param debug
     */
    public static void setDebug(boolean debug) {
        isDebug = debug;
    }

    public static void d(String msg) {
        if (isDebug && msg != null) {
            Log.d(TAG, msg);
        }
    }

    public static void d(String tag, String msg) {
        if (isDebug && msg != null) {
            Log.d(tag, msg);
        }
    }

    public static void e(String msg) {
        if (isDebug && msg != null) {
            Log.e(TAG, msg);
        }
    }

    public static void e(String tag, String msg) {
        if (isDebug && msg != null) {
            Log.e(tag, msg);
        }
    }

    public static Builder ld() {
        return Builder.start();
    }

    public static class Builder {
        StringBuilder builder;

        private Builder() {
            builder = new StringBuilder();
        }

        public Builder append(String msg) {
            builder.append(msg);
            builder.append("    ");
            return this;
        }

        public Builder append(Object obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(boolean obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(char obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(char[] obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(int obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(float obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(double obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(long obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }

        public Builder append(CharSequence obj) {
            builder.append(obj);
            builder.append("    ");
            return this;
        }


        public void showLog() {
            VLog.d(builder.toString());
        }

        public static Builder start() {
            return new Builder();
        }
    }
}