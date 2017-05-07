#include "bmputil.h"

void SaveBmpPicture(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[128];
    int y;

    //打开文件
    sprintf(szFilename, "/sdcard/AClip/reverse2/bmp%0d.bmp", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL) return;


    BITMAPFILEHEADER ptBitMapFileHeader;
    BITMAPINFOHEADER ptBitMapInfoHeader;

    memset(&ptBitMapFileHeader, 0, sizeof(BITMAPFILEHEADER));

    ptBitMapFileHeader.bfType = 0x4D42;
    ptBitMapFileHeader.bfSize = width * height * 3 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    ptBitMapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    ptBitMapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    ptBitMapInfoHeader.biWidth = width;
    ptBitMapInfoHeader.biHeight = height;
    ptBitMapInfoHeader.biPlanes = 1;
    ptBitMapInfoHeader.biBitCount = 24;
    ptBitMapInfoHeader.biCompression = 0;

    ptBitMapInfoHeader.biSizeImage = 0;
    ptBitMapInfoHeader.biXPelsPerMeter = width;
    ptBitMapInfoHeader.biYPelsPerMeter = height;
    ptBitMapInfoHeader.biClrUsed = 0;

    fwrite(&ptBitMapFileHeader, 1, sizeof(ptBitMapFileHeader), pFile);
    fwrite(&ptBitMapInfoHeader, 1, sizeof(ptBitMapInfoHeader), pFile);

    // Write pixel data
    for(y = height-1; y >= 0; y--)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

    // Close file
    fclose(pFile);
}