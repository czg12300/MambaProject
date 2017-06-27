
package com.framework;

/**
 * jni of video clip
 *
 * @author jake
 * @since 2016/12/30 下午5:18
 */

public class VideoClipJni {

    static {
        System.loadLibrary("native-lib");
        init();
    }

    /**
     * 初始化已经基本的东西，如初始化底层使用的线程池
     */
    private static native void init();

    /**
     * 销毁，如销毁底层使用的线程池
     */
    public static native void destroy();

    public static native int videoFilter(String path1, String outpath, String filterargs);

    public static native int videoTranscode(String path1, String outpath);

    public static native int videoMix(String srcFile1, String srcFile2, String outFile, int mix1,
            int mix2);

    public static native int audioDecode(String srcFile, String outFile, int decodetype);

    public static native int audioMix(String srcFile1, String srcFile2, String outPath, float vol1,
            float vol2);

    public static native int audioRate(String srcFile, String outFile, float rate,
            HandleProgressListener listener);

    public static native int audioVolumn(String srcFile1, String outFile, float vol1);

    public static native int reverse(String srcPath, String outFile);

    /**
     * 该方法实现视频合并，被合并的视频双方需要参数一致，合并后视频后缀与第一个视频文件相同
     *
     * @param srcFile1
     * @param srcFile2
     * @param outFile
     * @return
     */
    public static native int videoMerge(String srcFile1, String srcFile2, String outFile);

    /**
     * 完成将mp4视频解码后重新编码的一整套流程（用作功能演示）
     *
     * @param srcFile
     * @param outFile
     * @return
     */
    public static native int reEncode(String srcFile, String outFile);

    public static native int rotateVideo(String srcFile, String outFile, String rotate);

    public static native int videoRecordStart(String h264File, int width, int height, String rotate,
            int frameRate, long bitRate);

    public static native int videoRecording(byte[] data);

    public static native int videoRecordEnd();

    public static native int audioRecordStart(String outFile, int channels, int bitrate,
            int sample_rate);

    public static native int audioRecording(byte[] data, int length);

    public static native int audioRecordEnd();

    public static native int demuxingKeyFrame(String filePath, String picPathS);

    public static native int addWatermark(String watermarkCommand, String srcFile, String outFile);

    public static native int demuxing(String filePath, String h264, String aac);

    public static native int remuxing(String srcFile, String outFile);

    public static native int cutVideo(double start, double end, String inFile, String outFile);

    public static native int cutAudio(double start, double end, String inFile, String outFile);

    public static native int repeatVideoMoment(String inFile, String outFile, double second);

    public static native int relativeVideoMoment(String inFile, String outFile, double second);

    public static native int fastVideoSpeed(String inFile, String outFile, int rate,
            HandleProgressListener listener);

    public static native int slowVideoSpeed(String inFile, String outFile, int rate,
            HandleProgressListener listener);

    public static native int muxing(String h264, String aac, String mp4);

    public static native int muxingVideo(String video, String aac, String mp4);

    public static native String sayHello(String name);

    public static native void helloJni(String name);
}
