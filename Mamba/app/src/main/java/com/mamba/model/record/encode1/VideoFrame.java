package com.mamba.model.record.encode1;

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

    private VideoFrame() {
    }

    public static VideoFrame create(int width, int height, byte[] data, long timestamp) {
        VideoFrame frame = new VideoFrame();
        frame.width = width;
        frame.height = height;
        frame.data = data;
        frame.timestamp = timestamp;
        return frame;
    }
}
