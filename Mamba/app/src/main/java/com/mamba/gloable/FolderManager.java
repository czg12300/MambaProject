package com.mamba.gloable;

import android.os.Environment;

import com.framework.FrameworkApplication;
import com.framework.utils.FileUtil;
import com.mamba.AppApplication;

public class FolderManager {

    /**
     * SD卡目录
     */
    public static final String SDCARD_PATH = Environment.getExternalStorageDirectory()
            .getAbsolutePath();

    /**
     * 应用根目录
     */
    public static final String ROOT_FOLDER = SDCARD_PATH + "/Mamba/";

    /**
     * 缓存文件夹
     */
    public static String CACHE_FOLDER;

    /**
     * 图片缓存目录
     */
    public static final String CACHE_IMAGE_PATH = CACHE_FOLDER + "image/";

    /**
     * 视频缓存路径
     */
    public static final String CACHE_VIDEO_PATH = CACHE_FOLDER + "video/";

    public static final String CACHE_VIDEO_DRAFT = CACHE_VIDEO_PATH + "draft/";

    /**
     * 歌曲文件
     */
    public static final String SONGS_FOLDER = ROOT_FOLDER + "songs/";

    /**
     * 头像根目录
     */
    public static final String IMAGE_CACHE_FOLDER = CACHE_IMAGE_PATH + "avatar/";

    /**
     * 用户头像
     */
    public static final String USER_HEAD_ICON = IMAGE_CACHE_FOLDER + "user_icon.png";

    /**
     * 发布视频的在相册中的位置
     */
    public static final String PUBLISH_VIDEO_DIR = Environment
            .getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM).getAbsolutePath()
            + "/xg/";

    public static final String DOWNLOAD_FOLDER = ROOT_FOLDER + "Download/";

    /**
     * 初始化文件系统
     */
    public static void initSystemFolder() {
        boolean isSDCardAvailable = Environment.getExternalStorageState()
                .equals(Environment.MEDIA_MOUNTED);
        if (!isSDCardAvailable) {
            // 存储卡不可用，返回
            return;
        }
        CACHE_FOLDER = AppApplication.get().getApplicationContext().getCacheDir().getAbsolutePath();
        checkFolder(ROOT_FOLDER);
//        checkFolder(SONGS_FOLDER);
//        checkFolder(CACHE_FOLDER);
//        checkFolder(CACHE_IMAGE_PATH);
//        checkFolder(IMAGE_CACHE_FOLDER);
//        checkFolder(CACHE_VIDEO_PATH);
//        checkFolder(CACHE_VIDEO_DRAFT);
//        checkFolder(PUBLISH_VIDEO_DIR);
//        checkFolder(DOWNLOAD_FOLDER);
    }

    // 检查文件夹，不存在或文件夹版本号不同时，会重新创建
    private static void checkFolder(String folder) {
        if (!FileUtil.isFileExist(folder)) {
            FileUtil.createFolder(folder, false);
        }
    }

}