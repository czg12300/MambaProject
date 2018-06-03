package com.mamba.utils;


import android.hardware.Camera;
import android.os.Build;
import android.support.annotation.RequiresApi;

import java.util.ArrayList;
import java.util.List;

/**
 * 一些camera相关的简单的工具
 *
 * @author jake
 * @since 2017/4/27 下午5:36
 */

public final class CameraUtils {


    public static Size getLargeSize(List<Size> list, int width, int height) {
        if (list == null || list.size() == 0) {
            return new Size(width, height);
        }
        if (width > height) {
            int tempwidth = width;
            width = height;
            height = tempwidth;
        }
        // 存放宽高与屏幕宽高相同的size
        Size size = null;
        // 存放比率相同的最大size
        Size largeSameRatioSize = null;
        // 存放比率差距0.1的最大size
        Size largeRatioSize = null;
        float scrwhRatio = width * 1.0f / height * 1.0f;
        for (Size preSize : list) {
            float tempRatio = preSize.width * 1.0f / preSize.height * 1.0f;
            if (preSize.width < preSize.height) {
                tempRatio = preSize.width * 1.0f / preSize.height * 1.0f;
                if (preSize.width == width && preSize.height == height) {
                    size = preSize;
                    break;
                }
            } else if (preSize.width > preSize.height) {
                tempRatio = preSize.height * 1.0f / preSize.width * 1.0f;
                if (preSize.height == width && preSize.width == height) {
                    size = preSize;
                    break;
                }
            }

            if (tempRatio == scrwhRatio) {
                if (largeSameRatioSize == null) {
                    largeSameRatioSize = preSize;
                } else {
                    if (Math.abs(largeSameRatioSize.width - width) > Math.abs(preSize.width - width)) {
                        largeSameRatioSize = preSize;
                    }
                }
            }

            float ratioDistance = Math.abs(tempRatio - scrwhRatio);
            if (ratioDistance < 0.1) {
                if (largeRatioSize == null) {
                    largeRatioSize = preSize;
                } else {
                    if (Math.abs(largeRatioSize.width - width) > Math.abs(preSize.width - width)) {
                        largeRatioSize = preSize;
                    }
                }
            }
        }

        if (size != null) {
            return size;
        } else if (largeSameRatioSize != null) {
            if (Math.abs(largeSameRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                return largeSameRatioSize;
            } else if (largeRatioSize != null) {
                if (Math.abs(largeRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                    return largeRatioSize;
                } else {
                    return list.get(0);
                }
            } else {
                return list.get(0);
            }
        } else if (largeRatioSize != null) {
            if (Math.abs(largeRatioSize.width - width) < (width * 1.0f / 3.0f)) {
                return largeRatioSize;
            } else {
                return list.get(0);
            }
        } else {
            return list.get(0);
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public static List<Size> transArrayToList(android.util.Size[] data) {
        if (data != null) {
            List<Size> result = new ArrayList<>();
            for (android.util.Size size : data) {
                if (size != null) {

                    result.add(new Size(size.getWidth(), size.getHeight()));
                }
            }
            return result;
        }
        return null;
    }

    public static List<Size> transArrayToList(List<Camera.Size> data) {
        if (data != null) {
            List<Size> result = new ArrayList<>();
            for (Camera.Size size : data) {
                if (size != null) {
                    result.add(new Size(size.width, size.height));
                }
            }
            return result;
        }
        return null;
    }
    public static class Size {
        public int width;
        public int height;

        public Size() {
        }

        public Size(int width, int height) {
            this.width = width;
            this.height = height;
        }

        @Override
        public boolean equals(Object obj) {
            if (!(obj instanceof Size)) {
                return false;
            }
            Size s = (Size) obj;
            return width == s.width && height == s.height;
        }

        @Override
        public int hashCode() {
            return width * 32713 + height;
        }


        public int getHeight() {
            return height;
        }

        public int getWidth() {
            return width;
        }
    }
}
