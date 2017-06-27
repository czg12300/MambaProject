package com.framework.test;

import android.os.Environment;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * 缓存文件目录工具
 *
 * @author jake
 * @since 2017/2/10 上午11:21
 */

public class CacheDirUtil {
    public static final File ROOT = new File(Environment.getExternalStorageDirectory(), "AClip");
    public static final File CAMERA = new File(ROOT, "camera");
    public static final File CAMERA_JPEG = new File(CAMERA, "jpeg");
    public static final File CAMERA_VIDEO = new File(CAMERA, "video");

    static {
        checkDir(ROOT);
        checkDir(CAMERA);
        checkDir(CAMERA_JPEG);
        checkDir(CAMERA_VIDEO);
    }

    public static void checkDir(File dir) {
        if (dir != null && !dir.exists()) {
            dir.mkdirs();
        }
    }

    public static File getCameraJpeg() {
        File file = new File(CacheDirUtil.CAMERA_JPEG, "IMG_" + getCurrentTime() + ".jpg");
        checkFile(file);
        return file;
    }

    private static void checkFile(File file) {
        if (!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static File getCameraVideo() {
        File file = new File(CacheDirUtil.CAMERA_VIDEO, "VID_" + getCurrentTime() + ".flv");
        checkFile(file);
        return file;
    }

    private static String getCurrentTime() {
        return new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
    }
}
