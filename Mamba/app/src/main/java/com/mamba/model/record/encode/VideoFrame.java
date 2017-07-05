package com.mamba.model.record.encode;

/**
 * 视频帧数据
 *
 * @author jake
 * @since 2017/6/30 上午10:50
 */

public class VideoFrame {
    public byte[] data;
    public int width;
    public int height;
    public long timestamp;
    public int writeTimes = 0;
    public int rotate;

    private VideoFrame() {
    }

    public void setWriteTimes(int writeTimes) {
        this.writeTimes = writeTimes;
    }

    public static VideoFrame create(int width, int height, byte[] data, long timestamp, int rotate) {
        VideoFrame frame = new VideoFrame();
        frame.width = width;
        frame.height = height;
        frame.data = data;
        frame.timestamp = timestamp;
        frame.rotate = rotate;
        return frame;
    }
}
