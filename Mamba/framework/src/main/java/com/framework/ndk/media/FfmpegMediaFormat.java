package com.framework.ndk.media;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

/**
 * 视频格式属性
 *
 * @author jake
 * @since 2017/7/20 上午10:08
 */

public class FfmpegMediaFormat {
    //#define AVSEEK_FLAG_BACKWARD 1 ///< seek backward
//            #define AVSEEK_FLAG_BYTE     2 ///< seeking based on position in bytes
//            #define AVSEEK_FLAG_ANY      4 ///< seek to any frame, even non-keyframes
//            #define AVSEEK_FLAG_FRAME    8 ///< seeking based on frame number
    public static final int SEEK_MODE_BACKWARD = 1;
    public static final int SEEK_MODE_BYTE = 2;
    public static final int SEEK_MODE_ANY = 4;
    public static final int SEEK_MODE_FRAME = 6;
    public static final int CODEC_ID_H264 = 1;
    public static final int CODEC_ID_AAC = 2;
    public static final int CODE_TYPE_VIDEO = 1;
    public static final int CODE_TYPE_AUDIO = 2;
    public static final int AV_PIX_FMT_YUV420P = 1;
    public static final String KEY_WIDTH = "width";
    public static final String KEY_HEIGHT = "height";
    public static final String KEY_BIT_RATE = "bit_rate";
    public static final String KEY_CODEC_TYPE = "codec_type";
    public static final String KEY_SAMPLE_RATE = "sample_rate";
    public static final String KEY_CHANNELS = "channels";
    public static final String KEY_CHANNEL_LAYOUT = "channel_layout";
    public static final String KEY_CODEC_ID = "codec_id";
    public static final String KEY_FRAME_RATE = "frame_rate";
    public static final String KEY_CODEC = "is_encode";
    public static final String KEY_PIX_FMT = "pix_fmt";
    public static final String KEY_I_FRAME_INTERVAL = "key_i_frame_interval";
    private Map<String, Object> mMap;

    public FfmpegMediaFormat() {
        mMap = new HashMap<>();
    }

    public FfmpegMediaFormat(Map<String, Object> map) {
        mMap = map;
    }

    public static FfmpegMediaFormat createDecodeFormat() {
        FfmpegMediaFormat format = new FfmpegMediaFormat();
        format.setInteger(KEY_CODEC, 0);
        return format;
    }

    public static FfmpegMediaFormat createEncodeFormat() {
        FfmpegMediaFormat format = new FfmpegMediaFormat();
        format.setInteger(KEY_CODEC, 1);
        return format;
    }

    protected Map<String, Object> getMap() {
        return mMap;
    }

    /**
     * Sets the value of an integer key.
     */
    public final void setInteger(String name, int value) {
        mMap.put(name, Integer.valueOf(value));
    }

    /**
     * Sets the value of a long key.
     */
    public final void setLong(String name, long value) {
        mMap.put(name, Long.valueOf(value));
    }

    /**
     * Sets the value of a float key.
     */
    public final void setFloat(String name, float value) {
        mMap.put(name, new Float(value));
    }

    /**
     * Sets the value of a string key.
     */
    public final void setString(String name, String value) {
        mMap.put(name, value);
    }

    /**
     * Sets the value of a ByteBuffer key.
     */
    public final void setByteBuffer(String name, ByteBuffer bytes) {
        mMap.put(name, bytes);
    }

    /**
     * Returns the value of an integer key.
     */
    public int getInteger(String name) {
        int result = 0;
        Integer temp = (Integer) mMap.get(name);
        if (temp != null) {
            result = temp.intValue();
        }
        return result;
    }

    /**
     * Returns the value of an integer key, or the default value if the
     * key is missing or is for another type value.
     *
     * @hide
     */
//    public final int getInteger(String name, int defaultValue) {
//        try {
//            return getInteger(name);
//        } catch (NullPointerException e) { /* no such field */ } catch (ClassCastException e) { /* field of different type */ }
//        return defaultValue;
//    }

    /**
     * Returns the value of a long key.
     */
    public final long getLong(String name) {
        return ((Long) mMap.get(name)).longValue();
    }

    /**
     * Returns the value of a float key.
     */
    public final float getFloat(String name) {
        return ((Float) mMap.get(name)).floatValue();
    }

    /**
     * Returns the value of a string key.
     */
    public final String getString(String name) {
        return (String) mMap.get(name);
    }

    /**
     * Returns the value of a ByteBuffer key.
     */
    public final ByteBuffer getByteBuffer(String name) {
        return (ByteBuffer) mMap.get(name);
    }

    @Override
    public String toString() {
        return mMap.toString();
    }
}
