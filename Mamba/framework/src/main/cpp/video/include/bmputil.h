//
// Created by walljiang on 2017/04/20.
//

#ifndef GITXIUGE_BMPUTIL_H
#define GITXIUGE_BMPUTIL_H

extern "C" {
#include <libavutil/frame.h>
}
//
// Created by walljiang on 2017/04/20.
//
typedef struct tagBITMAPFILEHEADER { /* bmfh */
    unsigned short bfType;
    unsigned long  bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned long  bfOffBits;
}  __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER { /* bmih */
    unsigned long  biSize;
    unsigned long  biWidth;
    unsigned long  biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned long  biCompression;
    unsigned long  biSizeImage;
    unsigned long  biXPelsPerMeter;
    unsigned long  biYPelsPerMeter;
    unsigned long  biClrUsed;
    unsigned long  biClrImportant;
}  __attribute__ ((packed)) BITMAPINFOHEADER;


void SaveBmpPicture(AVFrame *pFrame, int width, int height, int iFrame);

#endif //GITXIUGE_BMPUTIL_H

