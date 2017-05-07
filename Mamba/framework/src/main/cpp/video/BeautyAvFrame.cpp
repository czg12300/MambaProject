//
// Created by jakechen on 2017/4/21.
//

#include "include/BeautyAvFrame.h"

#define div255(x) (x * 0.003921F)
#define abs(x) (x>=0 ? x:(-x))

const float YCbCrYRF = 0.299F;
const float YCbCrYGF = 0.587F;
const float YCbCrYBF = 0.114F;
const float YCbCrCbRF = -0.168736F;
const float YCbCrCbGF = -0.331264F;
const float YCbCrCbBF = 0.500000F;
const float YCbCrCrRF = 0.500000F;
const float YCbCrCrGF = -0.418688F;
const float YCbCrCrBF = -0.081312F;

const float RGBRYF = 1.00000F;
const float RGBRCbF = 0.0000F;
const float RGBRCrF = 1.40200F;
const float RGBGYF = 1.00000F;
const float RGBGCbF = -0.34414F;
const float RGBGCrF = -0.71414F;
const float RGBBYF = 1.00000F;
const float RGBBCbF = 1.77200F;
const float RGBBCrF = 0.00000F;

const int Shift = 20;
const int HalfShiftValue = 1 << (Shift - 1);

const int YCbCrYRI = (int) (YCbCrYRF * (1 << Shift) + 0.5);
const int YCbCrYGI = (int) (YCbCrYGF * (1 << Shift) + 0.5);
const int YCbCrYBI = (int) (YCbCrYBF * (1 << Shift) + 0.5);
const int YCbCrCbRI = (int) (YCbCrCbRF * (1 << Shift) + 0.5);
const int YCbCrCbGI = (int) (YCbCrCbGF * (1 << Shift) + 0.5);
const int YCbCrCbBI = (int) (YCbCrCbBF * (1 << Shift) + 0.5);
const int YCbCrCrRI = (int) (YCbCrCrRF * (1 << Shift) + 0.5);
const int YCbCrCrGI = (int) (YCbCrCrGF * (1 << Shift) + 0.5);
const int YCbCrCrBI = (int) (YCbCrCrBF * (1 << Shift) + 0.5);

const int RGBRYI = (int) (RGBRYF * (1 << Shift) + 0.5);
const int RGBRCbI = (int) (RGBRCbF * (1 << Shift) + 0.5);
const int RGBRCrI = (int) (RGBRCrF * (1 << Shift) + 0.5);
const int RGBGYI = (int) (RGBGYF * (1 << Shift) + 0.5);
const int RGBGCbI = (int) (RGBGCbF * (1 << Shift) + 0.5);
const int RGBGCrI = (int) (RGBGCrF * (1 << Shift) + 0.5);
const int RGBBYI = (int) (RGBBYF * (1 << Shift) + 0.5);
const int RGBBCbI = (int) (RGBBCbF * (1 << Shift) + 0.5);
const int RGBBCrI = (int) (RGBBCrF * (1 << Shift) + 0.5);

uint64_t *mIntegralMatrix;
uint64_t *mIntegralMatrixSqr;
uint8_t *mImageData_yuv;
uint8_t *mSkinMatrix;
uint64_t *mColumnSum;
uint64_t *mColumnSumSqr;

#define _USE_SMOOTH_ 1
#define _USE_WHITESKIN_ 1

void YCbCrToRGB(uint8_t *From, uint8_t *To, int length) {
    if (length < 1) return;
    int Red, Green, Blue;
    int Y, Cb, Cr;
    int i, offset;

    for (i = 0; i < length; i++) {
        offset = (i << 1) + i;
        Y = From[offset];
        Cb = From[offset + 1] - 128;
        Cr = From[offset + 2] - 128;
        Red = Y + ((RGBRCrI * Cr + HalfShiftValue) >> Shift);
        Green = Y + ((RGBGCbI * Cb + RGBGCrI * Cr + HalfShiftValue) >> Shift);
        Blue = Y + ((RGBBCbI * Cb + HalfShiftValue) >> Shift);
        if (Red > 255) Red = 255; else if (Red < 0) Red = 0;
        if (Green > 255) Green = 255; else if (Green < 0) Green = 0;
        if (Blue > 255) Blue = 255; else if (Blue < 0) Blue = 0;
        offset = i * 3;
        To[offset] = (uint8_t) Red;
        To[offset + 1] = (uint8_t) Green;
        To[offset + 2] = (uint8_t) Blue;
    }
}

void RGBToYCbCr(uint8_t *From, uint8_t *To, int length) {
    if (length < 1) return;
    int Red, Green, Blue;
    int i, offset;

    for (i = 0; i < length; i++) {
        offset = i * 3;
        Blue = From[offset + 2];
        Green = From[offset + 1];
        Red = From[offset + 0];
        offset = (i << 1) + i;
        To[offset] = (uint8_t) (
                (YCbCrYRI * Red + YCbCrYGI * Green + YCbCrYBI * Blue + HalfShiftValue) >> Shift);
        To[offset + 1] = (uint8_t) (128 + ((YCbCrCbRI * Red + YCbCrCbGI * Green + YCbCrCbBI * Blue +
                                            HalfShiftValue) >> Shift));
        To[offset + 2] = (uint8_t) (128 + ((YCbCrCrRI * Red + YCbCrCrGI * Green + YCbCrCrBI * Blue +
                                            HalfShiftValue) >> Shift));
    }
}


void _initIntegral(int width, int height) {
    int Length = width * height * 3;
    if (mIntegralMatrix == NULL)
        mIntegralMatrix = new uint64_t[Length];
    if (mIntegralMatrixSqr == NULL)
        mIntegralMatrixSqr = new uint64_t[Length];
    if (mColumnSum == NULL)
        mColumnSum = new uint64_t[width];
    if (mColumnSumSqr == NULL)
        mColumnSumSqr = new uint64_t[width];

    mColumnSum[0] = mImageData_yuv[0];
    mColumnSumSqr[0] = mImageData_yuv[0] * mImageData_yuv[0];

    mIntegralMatrix[0] = mColumnSum[0];
    mIntegralMatrixSqr[0] = mColumnSumSqr[0];

    for (int i = 1; i < width; i++) {
        mColumnSum[i] = mImageData_yuv[3 * i];
        mColumnSumSqr[i] = mImageData_yuv[3 * i] * mImageData_yuv[3 * i];

        mIntegralMatrix[i] = mColumnSum[i];
        mIntegralMatrix[i] += mIntegralMatrix[i - 1];
        mIntegralMatrixSqr[i] = mColumnSumSqr[i];
        mIntegralMatrixSqr[i] += mIntegralMatrixSqr[i - 1];
    }

    for (int i = 1; i < height; i++) {
        int offset = i * width;

        mColumnSum[0] += mImageData_yuv[3 * offset];
        mColumnSumSqr[0] += mImageData_yuv[3 * offset] * mImageData_yuv[3 * offset];

        mIntegralMatrix[offset] = mColumnSum[0];
        mIntegralMatrixSqr[offset] = mColumnSumSqr[0];

        for (int j = 1; j < width; j++) {
            mColumnSum[j] += mImageData_yuv[3 * (offset + j)];
            mColumnSumSqr[j] += mImageData_yuv[3 * (offset + j)] * mImageData_yuv[3 * (offset + j)];

            mIntegralMatrix[offset + j] = mIntegralMatrix[offset + j - 1] + mColumnSum[j];
            mIntegralMatrixSqr[offset + j] = mIntegralMatrixSqr[offset + j - 1] + mColumnSumSqr[j];
        }
    }
}


void _initSkinMatrix(const AVFrame *rgb_frame, int width, int height) {
    if (mSkinMatrix == NULL)
        mSkinMatrix = new uint8_t[width * height];

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int pos = i * width  + j;
            int r = rgb_frame->data[0][pos + 0];
            int g = rgb_frame->data[0][pos + 1];
            int b = rgb_frame->data[0][pos + 2];

            if (b > 50 && g > 40 && r > 20)
                mSkinMatrix[pos] = 255;
            else
                mSkinMatrix[pos] = 0;
        }
    }
}


void _startSkinSmooth(float smoothlevel, int width, int height, int radius) {
    if (mIntegralMatrix == NULL || mIntegralMatrixSqr == NULL) {
        return;
    }

    for (int i = 1; i < height; i++) {
        for (int j = 1; j < width; j++) {
            if (mSkinMatrix[i * width + j] == 255) {
                int offset = i * width + j;
                int iMax = i + radius >= height - 1 ? height - 1 : i + radius;
                int jMax = j + radius >= width - 1 ? width - 1 : j + radius;
                int iMin = i - radius <= 1 ? 1 : i - radius;
                int jMin = j - radius <= 1 ? 1 : j - radius;

                int squar = (iMax - iMin + 1) * (jMax - jMin + 1);
                int i4 = iMax * width + jMax;
                int i3 = (iMin - 1) * width + (jMin - 1);
                int i2 = iMax * width + (jMin - 1);
                int i1 = (iMin - 1) * width + jMax;

                float m = (mIntegralMatrix[i4]
                           + mIntegralMatrix[i3]
                           - mIntegralMatrix[i2]
                           - mIntegralMatrix[i1]) / squar;

                float v = (mIntegralMatrixSqr[i4]
                           + mIntegralMatrixSqr[i3]
                           - mIntegralMatrixSqr[i2]
                           - mIntegralMatrixSqr[i1]) / squar - m * m;
                float k = v / (v + smoothlevel);

                mImageData_yuv[offset * 3] = ceil(m - k * m + k * mImageData_yuv[offset * 3]);
            }
        }
    }
}


void _startWhiteSkin(AVFrame *rgb_frame, float whitenlevel, int width, int height) {
    float a = log(whitenlevel);
    LOGD("_startWhiteSkin whitenlevel %f width %d height %d", whitenlevel, width, height);
    for (int i = 0; i < height; i++) {
        uint8_t *LinePS = rgb_frame->data[0] + i * (width * 3);
        LOGD("LinePS ==null  ? %d   i %d",LinePS==NULL,i);
        for (int j = 0; j < width; j++) {

            int r = LinePS[0];
            int g = LinePS[1];
            int b = LinePS[2];

            if (a != 0) {
                r = 255 * (log((1.0 * r / 255.0) * (whitenlevel - 1) + 1) / a);
                g = 255 * (log((1.0 * g / 255.0) * (whitenlevel - 1) + 1) / a);
                b = 255 * (log((1.0 * b / 255.0) * (whitenlevel - 1) + 1) / a);
            }

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;

            LinePS[0] = r;
            LinePS[1] = g;
            LinePS[2] = b;

            LinePS += 3;
        }
    }
}


void beautifyAlgorithm(AVFrame *rgb_frame) {
    int width = rgb_frame->width;
    int height = rgb_frame->height;
    LOGD("rgbFrame beautifyAlgorithm width %d height %d",width, height);
#ifdef _USE_SMOOTH_
    int radius = width > height ? width * 0.02 : height * 0.02;
    int length = width * height * 3;
    if (mImageData_yuv == NULL)
        mImageData_yuv = new uint8_t[length];
    memset(mImageData_yuv, 0, length);
#endif

#ifdef _USE_WHITESKIN_
    LOGD("beautifyAlgorithm _startWhiteSkin");
    _startWhiteSkin(rgb_frame, 3.0, width, height);
#endif

#ifdef _USE_SMOOTH_
    RGBToYCbCr(rgb_frame->data[0], mImageData_yuv, width * height);
    _initSkinMatrix(rgb_frame, width, height);
    _initIntegral(width, height);
    _startSkinSmooth(500.0, width, height, radius);
    YCbCrToRGB(mImageData_yuv, (uint8_t*)rgb_frame->data[0], width * height);
#endif
}


void freeM() {
    if (mIntegralMatrix != NULL)
        delete[] mIntegralMatrix;
    if (mIntegralMatrixSqr != NULL)
        delete[] mIntegralMatrixSqr;
    if (mImageData_yuv != NULL)
        delete[] mImageData_yuv;
    if (mSkinMatrix != NULL)
        delete[] mSkinMatrix;
    if (mColumnSum != NULL)
        delete[] mColumnSum;
    if (mColumnSumSqr != NULL)
        delete[] mColumnSumSqr;
}