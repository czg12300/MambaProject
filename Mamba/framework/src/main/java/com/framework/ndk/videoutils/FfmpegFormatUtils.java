package com.framework.ndk.videoutils;

/**
 * 格式封装和分离器
 *
 * @author jake
 * @since 2017/5/10 上午10:17
 */

public class FfmpegFormatUtils {

    public static boolean format(String videoStream, String audioStream, String outFile) {
        return format(videoStream, audioStream, outFile, null);
    }

    /**
     * 封装格式，将视频码流和音频码流根据输出文件格式进行封装
     *
     * @param videoStream 视频码流文件
     * @param audioStream 音频码流文件
     * @param outFile     输出视频文件
     * @return
     */
    public static boolean format(String videoStream, String audioStream, String outFile, String videoRotate) {
        return muxer(videoStream, audioStream, outFile, videoRotate) >= 0;
    }

    /**
     * 封装格式，将视频码流根据输出文件格式进行封装
     *
     * @param videoStream 视频码流文件
     * @param outFile     输出视频文件
     * @return
     */
    public static boolean formatVideoStream(String videoStream, String outFile) {
        return formatVideoStream(videoStream, outFile, null);
    }

    public static boolean formatVideoStream(String videoStream, String outFile, String videoRotate) {
        return muxerVideoStream(videoStream, outFile, videoRotate) >= 0;
    }


    public static boolean transFormat(String srcFile, String outFile) {
        return transFormat(srcFile, outFile, null);
    }

    /**
     * 格式转换
     *
     * @param srcFile     源文件
     * @param outFile     输出文件
     * @param videoRotate 视频角度
     * @return
     */
    public static boolean transFormat(String srcFile, String outFile, String videoRotate) {
        return remuxer(srcFile, outFile, videoRotate) >= 0;
    }

    /**
     * 视频分离
     *
     * @param srcFile     源文件
     * @param videoStream 视频码率
     * @param audioStream 音频码流
     * @return
     */
    public static boolean separate(String srcFile, String videoStream, String audioStream) {
        return demuxer(srcFile, videoStream, audioStream) >= 0;
    }

    /**
     * 分离视频文件中的视频流
     *
     * @param srcFile
     * @param stream
     * @return
     */
    public static boolean separateVideo(String srcFile, String stream) {
        return demuxerVideo(srcFile, stream) >= 0;
    }

    /**
     * 分离视频文件中的音频流
     *
     * @param srcFile
     * @param stream
     * @return
     */
    public static boolean separateAudio(String srcFile, String stream) {
        return demuxerAudio(srcFile, stream) >= 0;
    }

    private static final native int muxer(String videoStream, String audioStream, String outFile, String videoRotate);

    private static final native int muxerVideoStream(String videoStream, String outFile, String videoRotate);


    private static final native int remuxer(String inFile, String outFile, String videoRotate);

    private static final native int demuxer(String file, String videoStream, String audioStream);

    private static final native int demuxerVideo(String file, String videoStream);

    private static final native int demuxerAudio(String file, String audioStream);
}
