package com.mamba.model.record.encode.audio;

/**
 * 音频帧数据
 *
 * @author jake
 * @since 2017/6/30 上午10:50
 */

public class AudioFrame {
    public byte[] data;
    public int size;

    private AudioFrame() {
    }


    public static AudioFrame create(byte[] data, int size) {
        AudioFrame frame = new AudioFrame();
        frame.size = size;
        frame.data = data;
        return frame;
    }
}
