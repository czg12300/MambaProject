//
// Created by jakechen on 2017/6/30.
//

#include "PixelTransUtils.h"


namespace video {

#define max(x, y)  (x>y?x:y)
#define min(x, y)  (x<y?x:y)
#define y(r, g, b)  (((66 * r + 129 * g + 25 * b + 128) >> 8) + 16)
#define u(r, g, b)  (((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128)
#define v(r, g, b)  (((112 * r - 94 * g - 18 * b + 128) >> 8) + 128)
#define color(x)  ((unsigned char)((x < 0) ? 0 : ((x > 255) ? 255 : x)))

#define RGBA_YUV420SP   0x00004012
#define BGRA_YUV420SP   0x00004210
#define RGBA_YUV420P    0x00014012
#define BGRA_YUV420P    0x00014210
#define RGB_YUV420SP    0x00003012
#define RGB_YUV420P     0x00013012
#define BGR_YUV420SP    0x00003210
#define BGR_YUV420P     0x00013210

/**
*   type 0-3位表示b的偏移量
*        4-7位表示g的偏移量
*        8-11位表示r的偏移量
*        12-15位表示rgba一个像素所占的byte
*        16-19位表示yuv的类型，0为420sp，1为420p
*/

    void rgbaToYuv(int width, int height, unsigned char *rgb, unsigned char *yuv, int type) {
        const int frameSize = width * height;
        const int yuvType = (type & 0x10000) >> 16;
        const int byteRgba = (type & 0x0F000) >> 12;
        const int rShift = (type & 0x00F00) >> 8;
        const int gShift = (type & 0x000F0) >> 4;
        const int bShift = (type & 0x0000F);
        const int uIndex = 0;
        const int vIndex = yuvType; //yuvType为1表示YUV420p,为0表示420sp

        int yIndex = 0;
        int uvIndex[2] = {frameSize, frameSize + frameSize / 4};

        unsigned char R, G, B, Y, U, V;
        unsigned int index = 0;
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                index = j * width + i;

                R = rgb[index * byteRgba + rShift] & 0xFF;
                G = rgb[index * byteRgba + gShift] & 0xFF;
                B = rgb[index * byteRgba + bShift] & 0xFF;

                Y = y(R, G, B);
                U = u(R, G, B);
                V = v(R, G, B);

                yuv[yIndex++] = color(Y);
                if (j % 2 == 0 && index % 2 == 0) {
                    yuv[uvIndex[uIndex]++] = color(U);
                    yuv[uvIndex[vIndex]++] = color(V);
                }
            }
        }
    }

    void rgbaToYuv(unsigned char *rgb, int width, int height, unsigned char *yuv) {
        rgbaToYuv(width, height, rgb, yuv, 1);
//        RGBAToI420()
    }

    int VideoStreamProcess(unsigned char *Src_data, unsigned char *Dst_data,
                           int src_width, int src_height,
                           bool EnableRotate, bool EnableMirror,
                           unsigned char *Dst_data_mirror, unsigned char *Dst_data_rotate,
                           int rotatemodel) {
        //src:NV12 video size
        int NV12_Size = src_width * src_height * 3 / 2;
        int NV12_Y_Size = src_width * src_height;

        //dst:YUV420 video size
        int I420_Size = src_width * src_height * 3 / 2;
        int I420_Y_Size = src_width * src_height;
        int I420_U_Size = (src_width >> 1) * (src_height >> 1);
        int I420_V_Size = I420_U_Size;

        // video format transformation process
        unsigned char *Y_data_Src = Src_data;
        unsigned char *UV_data_Src = Src_data + NV12_Y_Size;
        int src_stride_y = src_width;
        int src_stride_uv = src_width;

        unsigned char *Y_data_Dst = Dst_data;
        unsigned char *U_data_Dst = Dst_data + I420_Y_Size;
        unsigned char *V_data_Dst = Dst_data + I420_Y_Size + I420_U_Size;

        int Dst_Stride_Y = src_width;
        int Dst_Stride_U = src_width >> 1;
        int Dst_Stride_V = Dst_Stride_U;
//NV12ToI420
        libyuv::NV21ToI420(Y_data_Src, src_stride_y,
                           UV_data_Src, src_stride_uv,
                           Y_data_Dst, Dst_Stride_Y,
                           U_data_Dst, Dst_Stride_U,
                           V_data_Dst, Dst_Stride_V,
                           src_width, src_height);




        // video mirror process
        unsigned char *Y_data_Dst_mirror = Dst_data_mirror;
        unsigned char *U_data_Dst_mirror = Dst_data_mirror + I420_Y_Size;
        unsigned char *V_data_Dst_mirror = Dst_data_mirror + I420_Y_Size + I420_U_Size;
        int Dst_Stride_Y_mirror = src_width;
        int Dst_Stride_U_mirror = src_width >> 1;
        int Dst_Stride_V_mirror = Dst_Stride_U_mirror;

        if (EnableMirror) {
            libyuv::I420Mirror(Y_data_Dst, Dst_Stride_Y,
                               U_data_Dst, Dst_Stride_U,
                               V_data_Dst, Dst_Stride_V,
                               Y_data_Dst_mirror, Dst_Stride_Y_mirror,
                               U_data_Dst_mirror, Dst_Stride_U_mirror,
                               V_data_Dst_mirror, Dst_Stride_V_mirror,
                               src_width, src_height);
        }

        //video rotate process
        if (EnableRotate) {
            int Dst_Stride_Y_rotate;
            int Dst_Stride_U_rotate;
            int Dst_Stride_V_rotate;
            unsigned char *Y_data_Dst_rotate = Dst_data_rotate;
            unsigned char *U_data_Dst_rotate = Dst_data_rotate + I420_Y_Size;
            unsigned char *V_data_Dst_rotate = Dst_data_rotate + I420_Y_Size + I420_U_Size;

            if (rotatemodel == libyuv::kRotate90 || rotatemodel == libyuv::kRotate270) {
                Dst_Stride_Y_rotate = src_height;
                Dst_Stride_U_rotate = src_height >> 1;
                Dst_Stride_V_rotate = Dst_Stride_U_rotate;
            } else {
                Dst_Stride_Y_rotate = src_width;
                Dst_Stride_U_rotate = src_width >> 1;
                Dst_Stride_V_rotate = Dst_Stride_U_rotate;
            }

            if (EnableMirror) {
                libyuv::I420Rotate(Y_data_Dst_mirror, Dst_Stride_Y_mirror,
                                   U_data_Dst_mirror, Dst_Stride_U_mirror,
                                   V_data_Dst_mirror, Dst_Stride_V_mirror,
                                   Y_data_Dst_rotate, Dst_Stride_Y_rotate,
                                   U_data_Dst_rotate, Dst_Stride_U_rotate,
                                   V_data_Dst_rotate, Dst_Stride_V_rotate,
                                   src_width, src_height,
                                   (libyuv::RotationMode) rotatemodel);
            } else {
                libyuv::I420Rotate(Y_data_Dst, Dst_Stride_Y,
                                   U_data_Dst, Dst_Stride_U,
                                   V_data_Dst, Dst_Stride_V,
                                   Y_data_Dst_rotate, Dst_Stride_Y_rotate,
                                   U_data_Dst_rotate, Dst_Stride_U_rotate,
                                   V_data_Dst_rotate, Dst_Stride_V_rotate,
                                   src_width, src_height,
                                   (libyuv::RotationMode) rotatemodel);
            }
        }
        return 0;
    }

    void
    yuv420spToYuv420p(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
                      int dest_width,
                      int dest_height) {
        uint8 *src_y = yuv;
        int src_stride_y = width;
        uint8 *src_vu = yuv + width * height;
        int src_stride_vu = width;

        LOGD("yuv420spToYuv420p start");
        int len = width * height;
        int size = len * 3 / 2;
        uint8 *output = new uint8[size];

        uint8 *dst_y = yuv420p;
        int dst_stride_y = width;
        uint8 *dst_u = dst_y + len;
        int dst_stride_u = width >> 1;
        uint8 *dst_v = dst_u + (width >> 1) * (height >> 1);;
        int dst_stride_v = dst_stride_u;
        LOGD("yuv420spToYuv420p start");
        NV21ToI420(src_y, src_stride_y,
                   src_vu, src_stride_vu,
                   dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   width, height);
        LOGD("yuv420spToYuv420p NV12ToI420Rotate");
        uint8 *output1 = new uint8[size];
        int len1 = dest_width * dest_height;
        uint8 *dst_y1 = output1;
        int dst_stride_y1 = dest_width;
        uint8 *dst_u1 = dst_y1 + (len1);
        int dst_stride_u1 = dest_width / 2;
        uint8 *dst_v1 = dst_u1 + (len1) / 4;
        int dst_stride_v1 = dest_width / 2;
        I420Rotate(dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   dst_y1, dst_stride_y1,
                   dst_u1, dst_stride_u1,
                   dst_v1, dst_stride_v1,
                   width, height,
                   kRotate90
        );
        delete[]output;

        uint8 *src_y2 = yuv420p;
        int src_stride_y2 = dest_width;
        uint8 *src_vu2 = yuv420p + dest_width * dest_height;
        int src_stride_vu2 = dest_width;

        I420ToNV12(
                dst_y1, dst_stride_y1,
                dst_u1, dst_stride_u1,
                dst_v1, dst_stride_v1,
                src_y2, src_stride_y2,
                src_vu2, src_stride_vu2,
                dest_width, dest_height
        );
        LOGD("yuv420spToYuv420p I420Scale");
        delete[]output1;
        LOGD("yuv420spToYuv420p success");
    }

    void
    nv21ToYv12(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
               int dest_width,
               int dest_height, int rotate) {
        uint8 *src_y = yuv;
        int src_stride_y = width;
        uint8 *src_vu = yuv + width * height;
        int src_stride_vu = width;

        LOGD("yuv420spToYuv420p start");
        int len = width * height;
        int size = len * 3 / 2;
        uint8 *output = new uint8[size];

        uint8 *dst_y = yuv420p;
        int dst_stride_y = width;
        uint8 *dst_u = dst_y + len;
        int dst_stride_u = width >> 1;
        uint8 *dst_v = dst_u + (width >> 1) * (height >> 1);;
        int dst_stride_v = dst_stride_u;
        LOGD("yuv420spToYuv420p start");
        NV21ToI420(src_y, src_stride_y,
                   src_vu, src_stride_vu,
                   dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   width, height);
        LOGD("yuv420spToYuv420p NV12ToI420Rotate");
        uint8 *output1 = new uint8[size];
        int len1 = dest_width * dest_height;
        uint8 *dst_y1 = output1;
        int dst_stride_y1 = dest_width;
        uint8 *dst_u1 = dst_y1 + (len1);
        int dst_stride_u1 = dest_width / 2;
        uint8 *dst_v1 = dst_u1 + (len1) / 4;
        int dst_stride_v1 = dest_width / 2;
        RotationMode mode = kRotate0;
        switch (rotate) {
            case 0:
                mode = kRotate0;
                break;
            case 90:
                mode = kRotate90;
                break;
            case 180:
                mode = kRotate180;
                break;
            case 270:
                mode = kRotate270;
                break;
        }
        I420Rotate(dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   dst_y1, dst_stride_y1,
                   dst_u1, dst_stride_u1,
                   dst_v1, dst_stride_v1,
                   width, height,
                   mode
        );
        delete[]output;

        uint8 *src_y2 = yuv420p;
        int src_stride_y2 = dest_width;
        uint8 *src_vu2 = yuv420p + dest_width * dest_height;
        int src_stride_vu2 = dest_width;

        I420ToNV12(
                dst_y1, dst_stride_y1,
                dst_u1, dst_stride_u1,
                dst_v1, dst_stride_v1,
                src_y2, src_stride_y2,
                src_vu2, src_stride_vu2,
                dest_width, dest_height
        );
        LOGD("yuv420spToYuv420p I420Scale");
        delete[]output1;
        LOGD("yuv420spToYuv420p success");
    }

    void
    nv21ToI420(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
               int dest_width,
               int dest_height, int rotate) {
        int lenSrc = width * height;
        int sizeSrc = lenSrc * 3 / 2;
        uint8 *src_y = yuv;
        int src_stride_y = width;
        uint8 *src_vu = yuv + lenSrc;
        int src_stride_vu = width;
        LOGD("yuv420spToYuv420p start");

        uint8 *output = new uint8[sizeSrc];
        uint8 *dst_y = output;
        int dst_stride_y = width;
        uint8 *dst_u = dst_y + lenSrc;
        int dst_stride_u = width >> 1;
        uint8 *dst_v = dst_u + (width >> 1) * (height >> 1);
        int dst_stride_v = dst_stride_u;
        NV21ToI420(src_y, src_stride_y,
                   src_vu, src_stride_vu,
                   dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   width, height);
        LOGD("yuv420spToYuv420p NV12ToI420Rotate");
        int dstLen = dest_width * dest_height;
        uint8 *dst_y1 = yuv420p;
        int dst_stride_y1 = dest_width;
        uint8 *dst_u1 = dst_y1 + dstLen;
        int dst_stride_u1 = dest_width >> 1;
        uint8 *dst_v1 = dst_u1 + (dest_width >> 1) * (dest_height >> 1);
        int dst_stride_v1 = dst_stride_u1;
        RotationMode mode = kRotate0;
        switch (rotate) {
            case 0:
                mode = kRotate0;
                break;
            case 90:
                mode = kRotate90;
                break;
            case 180:
                mode = kRotate180;
                break;
            case 270:
                mode = kRotate270;
                break;
        }
        I420Rotate(dst_y, dst_stride_y,
                   dst_u, dst_stride_u,
                   dst_v, dst_stride_v,
                   dst_y1, dst_stride_y1,
                   dst_u1, dst_stride_u1,
                   dst_v1, dst_stride_v1,
                   width, height,
                   mode
        );
        delete[]output;
        LOGD("yuv420spToYuv420p success");
    }
}
