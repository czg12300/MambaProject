package com.mamba.model.play;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.view.Surface;

import com.framework.ndk.media.FfmpegCodec;
import com.framework.ndk.media.FfmpegMediaExtractor;
import com.framework.ndk.media.FfmpegMediaFormat;
import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.VLog;

import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author jake
 * @since 2017/7/18 上午10:52
 */

public class MediaCodecDecodeSurfaceTest implements Runnable, Test {
    private static final String MIME_TYPE = MediaFormat.MIMETYPE_VIDEO_AVC;
    private FfmpegCodec mediaCodec;
    private MediaExtractor extractor1;
    private FfmpegMediaExtractor extractor;
    private int width;
    private int height;
    private volatile boolean isStop = true;
    private DataOutputStream mOutput;
    private String filePath;
    Surface surface;
    private MediaCodec encoder;
    private int frameRate;
    private int frameIndex;
    private String h264;
    private String result;
    boolean isA = false;

    public void toggle() {
        isA = !isA;
    }

    public void setSurface(Surface surface) {
        this.surface = surface;
    }

    public void start(String filePath) {
        if (isStop) {
            VLog.d("mpe4 file =" + filePath);
            this.filePath = filePath;
            new Thread(this).start();
            isStop = false;

        }
    }

    private String getOutYuv(String name) {
        String file = FolderManager.ROOT_FOLDER + name + ".h264";
        VLog.d("yuv file =" + file);
        return file;
    }

    @Override
    public void run() {
        VLog.d("run");
        try {
            prepare();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
        if (listener != null) {
            listener.onStart();
        }
        loopDecode();
        try {
            mOutput.flush();
            mOutput.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        FfmpegFormatUtils.formatVideoStream(h264, result);
        release();
        isStop = true;
        if (listener != null) {
            listener.onFinish();
        }
    }

    private void release() {
        if (mediaCodec != null) {
            mediaCodec.release();
            mediaCodec = null;
        }
        if (encoder != null) {
            encoder.stop();
            encoder.release();
            encoder = null;
        }
        if (isA) {
            if (extractor1 != null) {
                extractor1.release();
                extractor1 = null;
            }
        } else {
            if (extractor != null) {
                extractor.release();
                extractor = null;
            }
        }

    }

    long last;

    private void loopDecode() {
        VLog.d("loopDecode   start");
        boolean isEOS = false;
        while (true) {
            if (!isEOS) {
                int sampleSize = -1;
                long timestamp = -1;
                byte[] data = null;
                if (isA) {
                    ByteBuffer byteBuffer = ByteBuffer.allocate(101111);
                    sampleSize = extractor1.readSampleData(byteBuffer, 0);
                    timestamp = extractor1.getSampleTime();
                    if (sampleSize > 0) {
                        data = new byte[sampleSize];
                        byteBuffer.get(data);
                    }
                } else {
                    data = extractor.readSampleData();
                    sampleSize = extractor.getSampleDataSize();
                    timestamp = extractor.getTimestamp();
                }

                VLog.d("loopDecode  readSampleData end sampleSize" + sampleSize);
                VLog.d("loopDecode  readSampleData end timestamp" + timestamp);
                if (sampleSize <= 0) {
                    isEOS = true;
                } else {
                    if (data != null) {
                        byte[] dest = mediaCodec.codec(data,sampleSize);
                        try {
                            mOutput.write(dest);
                        } catch (Exception e) {
                            e.printStackTrace();
                        } catch (Error e) {
                            e.printStackTrace();
                        }
//                        encode(dest);
                    }
                    if (isA)
                        extractor1.advance();
                }
            }
        }
    }

    boolean is = false;

    private void encode(byte[] data) {
        if (is) {
            is = false;
            return;
        } else {
            is = true;
        }
        ByteBuffer[] inputBuffers = encoder.getInputBuffers();
        ByteBuffer[] outputBuffers = encoder.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int inIndex = encoder.dequeueInputBuffer(10);
        if (inIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inIndex];
            inputBuffer.clear();
            inputBuffer.put(data);
            encoder.queueInputBuffer(inIndex, 0, data.length, computePresentationTime(frameIndex++), 0);
        }
        int outIndex = encoder.dequeueOutputBuffer(info, 10);
        if (outIndex >= 0) {
            while (outIndex >= 0) {
                VLog.d("while (outputBufferIndex >= 0)");
                ByteBuffer outputBuffer = outputBuffers[outIndex];
                byte[] outData = new byte[info.size];
                outputBuffer.get(outData);
                try {
                    mOutput.write(outData);
                } catch (Exception e) {
                    e.printStackTrace();
                } catch (Error e) {
                    e.printStackTrace();
                }
                encoder.releaseOutputBuffer(outIndex, false);
                outIndex = encoder.dequeueOutputBuffer(info, 10);
            }
        }
    }

    private void prepare() throws IOException {
        if (isA) {
            extractor1 = new MediaExtractor();
            extractor1.setDataSource(filePath);
            int numTracks = extractor1.getTrackCount();
            MediaFormat fformat1 = null;
            for (int i = 0; i < numTracks; i++) {
                fformat1 = extractor1.getTrackFormat(i);
                if (fformat1.getString(MediaFormat.KEY_MIME).startsWith("video")) {
                    extractor1.selectTrack(i);
                    break;
                }
            }
            width = fformat1.getInteger(MediaFormat.KEY_WIDTH);
            height = fformat1.getInteger(MediaFormat.KEY_HEIGHT);
            frameRate = fformat1.getInteger(MediaFormat.KEY_FRAME_RATE);
        } else {
            extractor = new FfmpegMediaExtractor();
            extractor.setDataSource(filePath);
            int numTracks = extractor.getTrackCount();
            FfmpegMediaFormat fformat = null;
            for (int i = 0; i < numTracks; ++i) {
                fformat = extractor.getTrackFormat(i);
                if (fformat.getInteger(FfmpegMediaFormat.KEY_CODEC_TYPE) == FfmpegMediaFormat.CODE_TYPE_VIDEO) {
                    extractor.selectTrack(i);
                    break;
                }
            }
            if (fformat == null) {
                throw new IllegalArgumentException("can't find format!");
            }
            width = fformat.getInteger(FfmpegMediaFormat.KEY_WIDTH);
            height = fformat.getInteger(FfmpegMediaFormat.KEY_HEIGHT);
            frameRate = fformat.getInteger(FfmpegMediaFormat.KEY_FRAME_RATE);
        }
        VLog.d("isA=" + isA + "  prepare   width=" + width + "  height=" + height + "   frameRate=" + frameRate);
        FfmpegMediaFormat dFormat = FfmpegMediaFormat.createDecodeFormat();
        dFormat.setInteger(FfmpegMediaFormat.KEY_CODEC_ID, FfmpegMediaFormat.CODEC_ID_H264);
        dFormat.setInteger(FfmpegMediaFormat.KEY_CODEC_TYPE, FfmpegMediaFormat.CODE_TYPE_VIDEO);
        dFormat.setInteger(FfmpegMediaFormat.KEY_HEIGHT, height);
        dFormat.setInteger(FfmpegMediaFormat.KEY_WIDTH, width);
        dFormat.setInteger(FfmpegMediaFormat.KEY_FRAME_RATE, frameRate);
        mediaCodec = new FfmpegCodec();
        mediaCodec.configure(dFormat);
        mediaCodec.start();
        VLog.d(dFormat.toString());
        h264 = filePath + ".h264";
        result = filePath + "result.mp4";
        FileUtil.createFile(h264, true);
        frameIndex = 0;
        mOutput = new DataOutputStream(new FileOutputStream(h264));
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE, width, height);
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, frameRate);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, (int) (2.0 * 1024 * 1024));
        VLog.d(mediaFormat.toString());
        encoder = MediaCodec.createEncoderByType(MIME_TYPE);
        encoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        encoder.start();
//        FfmpegMediaFormat format = FfmpegMediaFormat.createEncodeFormat();
//        format.setInteger(FfmpegMediaFormat.KEY_CODEC_ID, FfmpegMediaFormat.CODEC_ID_H264);
//        format.setInteger(FfmpegMediaFormat.KEY_CODEC_TYPE, FfmpegMediaFormat.CODE_TYPE_VIDEO);
//        format.setInteger(FfmpegMediaFormat.KEY_HEIGHT, height);
//        format.setInteger(FfmpegMediaFormat.KEY_FRAME_RATE, frameRate);
//        format.setInteger(FfmpegMediaFormat.KEY_PIX_FMT, FfmpegMediaFormat.AV_PIX_FMT_YUV420P);
//        format.setInteger(FfmpegMediaFormat.KEY_I_FRAME_INTERVAL, 1);
//        format.setInteger(FfmpegMediaFormat.KEY_BIT_RATE, (int) (2.0 * 1024 * 1024));
//        VLog.d(format.toString());
//        encoder = new FfmpegCodec();
//        encoder.configure(format);
    }

    private long computePresentationTime(int frameIndex) {
        return 132 + frameIndex * 1000000 / frameRate;
    }

    private Listener listener;

    public void setListener(Listener listener) {
        this.listener = listener;
    }

}
