package com.framework.ndk.videoutils;

/**
 * 音视频的裁剪和拼接工具
 *
 * @author jake
 * @since 2017/5/12 上午11:46
 */

public class FfmpegCutMergeUtils {
    public static boolean cutAudio(String srcFile, long startMillis, long endMillis, String outFile) {
        return nativeCutAudio(srcFile, startMillis, endMillis, outFile) >= 0;
    }

    public static boolean cutVideo(String srcFile, long startMillis, long endMillis, String outFile) {
        return nativeCutVideo(srcFile, startMillis, endMillis, outFile) >= 0;
    }

    public static boolean mergeAudio(String outFile, String... files) {
        return nativeMerge(files, outFile) >= 0;
    }

    public static boolean mergeVideo(String outFile, String... files) {
        return nativeMerge(files, outFile) >= 0;
    }

    private static final native int nativeCutAudio(String srcFile, long startMillis, long endMillis, String outFile);

    private static final native int nativeCutVideo(String srcFile, long startMillis, long endMillis, String outFile);


    private static final native int nativeMerge(String[] files, String outFile);
}
