package com.framework.ndk.videoutils;

/**
 * 图像像素格式转换
 *
 * @author jake
 * @since 2017/6/30 上午10:36
 */

public class PixelTransUtils {
    public static native void rgbaToYuv(byte[] src,int width,int height,byte[] dest);
}
