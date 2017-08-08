//
// Created by jakechen on 2017/7/20.
//

#ifndef MAMBA_MEDIAEXTRACTOR_H
#define MAMBA_MEDIAEXTRACTOR_H

#include <map>;
#include "Log.h"
#include <iostream>
#include "MediaFormat.h"

using namespace std;
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}
namespace video {

    static int convert_sps_pps(const uint8_t *p_buf, size_t i_buf_size,
                               uint8_t *p_out_buf, size_t i_out_buf_size,
                               size_t *p_sps_pps_size, size_t *p_nal_size) {
        // int i_profile;
        uint32_t i_data_size = i_buf_size, i_nal_size, i_sps_pps_size = 0;
        unsigned int i_loop_end;

        /* */
        if (i_data_size < 7) {
            LOGE("Input Metadata too small");
            return -1;
        }

        /* Read infos in first 6 bytes */
        // i_profile    = (p_buf[1] << 16) | (p_buf[2] << 8) | p_buf[3];
        if (p_nal_size)
            *p_nal_size = (p_buf[4] & 0x03) + 1;
        p_buf += 5;
        i_data_size -= 5;

        for (unsigned int j = 0; j < 2; j++) {
            /* First time is SPS, Second is PPS */
            if (i_data_size < 1) {
                LOGE("PPS too small after processing SPS/PPS %u",
                      i_data_size);
                return -1;
            }
            i_loop_end = p_buf[0] & (j == 0 ? 0x1f : 0xff);
            p_buf++;
            i_data_size--;

            for (unsigned int i = 0; i < i_loop_end; i++) {
                if (i_data_size < 2) {
                    LOGE("SPS is too small %u", i_data_size);
                    return -1;
                }

                i_nal_size = (p_buf[0] << 8) | p_buf[1];
                p_buf += 2;
                i_data_size -= 2;

                if (i_data_size < i_nal_size) {
                    LOGE("SPS size does not match NAL specified size %u",
                          i_data_size);
                    return -1;
                }
                if (i_sps_pps_size + 4 + i_nal_size > i_out_buf_size) {
                    LOGE("Output SPS/PPS buffer too small");
                    return -1;
                }

                p_out_buf[i_sps_pps_size++] = 0;
                p_out_buf[i_sps_pps_size++] = 0;
                p_out_buf[i_sps_pps_size++] = 0;
                p_out_buf[i_sps_pps_size++] = 1;

                memcpy(p_out_buf + i_sps_pps_size, p_buf, i_nal_size);
                i_sps_pps_size += i_nal_size;

                p_buf += i_nal_size;
                i_data_size -= i_nal_size;
            }
        }

        *p_sps_pps_size = i_sps_pps_size;

        return 0;
    }


/* Convert H.264 NAL format to annex b in-place */
    typedef struct H264ConvertState {
        uint32_t nal_len;
        uint32_t nal_pos;
    } H264ConvertState;

    static void convert_h264_to_annexb(uint8_t *p_buf, size_t i_len,
                                       size_t i_nal_size,
                                       H264ConvertState *state) {
        if (i_nal_size < 3 || i_nal_size > 4)
            return;

        /* This only works for NAL sizes 3-4 */
        while (i_len > 0) {
            if (state->nal_pos < i_nal_size) {
                unsigned int i;
                for (i = 0; state->nal_pos < i_nal_size && i < i_len; i++, state->nal_pos++) {
                    state->nal_len = (state->nal_len << 8) | p_buf[i];
                    p_buf[i] = 0;
                }
                if (state->nal_pos < i_nal_size)
                    return;
                p_buf[i - 1] = 1;
                p_buf += i;
                i_len -= i;
            }
            if (state->nal_len > INT_MAX)
                return;
            if (state->nal_len > i_len) {
                state->nal_len -= i_len;
                return;
            } else {
                p_buf += state->nal_len;
                i_len -= state->nal_len;
                state->nal_len = 0;
                state->nal_pos = 0;
            }
        }
    }

    class MediaExtractor {
    private:
        AVFormatContext *fmt_ctx = NULL;

        int duration = 0;
        int trackIndex = AVERROR_STREAM_NOT_FOUND;
        AVPacket packet;


    public:
        MediaExtractor();

        int _setDataSource(const char *file);

        void _seekTo(int timeUs, int mode);

        int _getTrackCount();

        MediaFormat *_getTrackFormat(int index);

        int _getTimestamp();

        int _getSampleDataSize();

        uint8_t *_readSampleData(int *size);

        void _selectTrack(int index);

        int _getTrackIndex();

        int _getDuration();

        void _release();

    };
}


#endif //MAMBA_MEDIAEXTRACTOR_H
