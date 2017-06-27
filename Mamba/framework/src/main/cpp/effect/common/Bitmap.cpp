//
// Created by yongfali on 2016/3/24.
//

#include "include/Bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "GPUImageCommon.h"
#include "include/Log.h"

namespace e
{
	#ifndef BI_RGB
	#   define BI_RGB 0
	#endif

	typedef unsigned char byte;
	typedef unsigned short uint16;
	typedef unsigned int uint32;

	#pragma pack(push, 1)
	typedef struct _RGBQUAD {
		byte red;
		byte green;
		byte blue;
		byte reserved;
	} RGBQUAD;

	typedef struct _BitmapHeader {
		uint16 bfType;
		uint32 bfSize;
		uint16 bfReserved1;
		uint16 bfReserved2;
		uint32 bfOffBits;
		uint32 biSize;
		uint32 biWidth;
		uint32 biHeight;
		uint16 biPlanes;
		uint16 biBitCount;
		uint32 biCompression;
		uint32 biSizeImage;
		uint32 biXPelsPerMeter;
		uint32 biYPelsPerMeter;
		uint32 biClrUsed;
		uint32 biClrImportant;
	} BITMAPHEADER;
	#pragma pack(pop)

	inline bool read(void* buffer, int size, FILE* fp)
	{
		return fread(buffer, 1, size, fp) == (size_t)size;
	}

	inline bool write(void* buffer, int size, FILE* fp)
	{
		return fwrite(buffer, 1, size, fp) == (size_t)size;
	}

	inline bool skip(int size, int flag, FILE* fp)
	{
		return fseek(fp, size, flag) == 0;
	}

	bool I420sp::Save(const char* fileName, int width, int height, void* bits, int size)
	{
		FILE* fp = fopen(fileName, "wb");
		if (fp == NULL) return false;
		bool ret = write(bits, size, fp);
		fclose(fp);
		return ret;
	}

	bool Bitmap::Save(const char* fileName, int width, int height, int bitCount, void* bits, int size)
	{
		FILE*  fp = NULL;
		RGBQUAD* quad = NULL;
		bool result = false;
		do
		{
			if ((fp = fopen(fileName, "wb")) == NULL)
			{
				LOGE("open file failed:%s", fileName);
				break;
			}

			const uint32 lineBytes = WidthBytes(width * bitCount);
			const uint32 imageSize = lineBytes * height;
			const uint32 paletteSize = (1 << bitCount) * sizeof(RGBQUAD);

			BITMAPHEADER header = { 0 };
			header.bfType = 0x4D42;
			header.bfSize = 0;
			header.bfReserved1 = 0;
			header.bfReserved2 = 0;
			header.bfOffBits = 0;

			if (bitCount == 8)
			{
				header.bfOffBits = 54 + paletteSize;
				header.bfSize = 54 + paletteSize + imageSize;
			}
			else if (bitCount == 24 || bitCount == 32)
			{
				header.bfOffBits = 54;
				header.bfSize = 54 + imageSize;
			}

			header.biSize = 40;
			header.biWidth = width;
			header.biHeight = height;
			header.biPlanes = 1;
			header.biBitCount = (uint16)bitCount;
			header.biCompression = BI_RGB;
			header.biSizeImage = imageSize;
			header.biXPelsPerMeter = 3780;
			header.biYPelsPerMeter = 3780;
			header.biClrUsed = 0;
			header.biClrImportant = 0;
#if 0
			LOGD("bitmap header save info:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
			     , header.biSize, header.biWidth, header.biHeight, header.biPlanes, header.biBitCount
			     , header.biCompression, header.biSizeImage, header.biXPelsPerMeter
			     , header.biYPelsPerMeter, header.biClrUsed, header.biClrImportant, lineBytes);
#endif
			//write bmp header
			if (!write(&header, sizeof(header), fp))
			{
				LOGE("write bitmap header failed:%s", fileName);
				break;
			}

			//write quad
			if (bitCount == 8)
			{
				int allocSize = sizeof(RGBQUAD) * (1 << bitCount);
				quad = (RGBQUAD*)malloc(allocSize);
				if (quad == NULL) goto _error;

				for (int i = 0; i < (1 << bitCount); i++)
				{
					quad[i].red = i;
					quad[i].green = i;
					quad[i].blue = i;
				}

				if (!write(quad, sizeof(RGBQUAD) * (1 << bitCount), fp))
				{
					LOGE("write bitmap palette failed:%s", fileName);
					goto _error;
				}
			}
			//需要将文件中的数据上下倒转过来
			byte* p = (byte*)bits + (height - 1) * lineBytes;
			for (int i = 0; i < height; i++)
			{
				if (!write(p, lineBytes, fp))
				{
					LOGE("write file failed:%s", fileName);
					goto _error;
				}
				p -= lineBytes;
			}
			result = true;
			//LOGD("write bitmap bits ok:%s", fileName);
		} while (0);
	_error:
		if (fp) fclose(fp);
		if (quad) free(quad);
		return result;
	}
	
#ifndef min
#   define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#   define max(a,b) ((a)<(b)?(b):(a))
#endif	

    void DrawPoint(int x
	    , int y
	    , int nPenSize
	    , int nColor
	    , void* pData
	    , int nSize
	    , int nWidth
	    , int nHeight
	    , int nBitCount)
    {
    	limit(x, 0, nWidth-1);
    	limit(y, 0, nHeight-1);

    	byte b = (nColor & 0x000000ff);
		byte g = (nColor & 0x0000ff00) >> 8;
		byte r = (nColor & 0x00ff0000) >> 16;
		const int nLineSize = WidthBytes(nBitCount*nWidth);
		const int nPixelSize = nBitCount / 8;	

		byte* p = (byte*)pData + y * nLineSize + x * nPixelSize;
		
		*(p + 0) = b;
		*(p + 1) = g;
		*(p + 2) = r;
    }

	void DrawRect(int x
	    , int y
	    , int w
	    , int h
	    , int nPenSize
	    , int nColor
	    , void* pData
	    , int nSize
	    , int nWidth
	    , int nHeight
	    , int nBitCount)
	{
		int x0 = x, y0 = y, x1 = x0 + w, y1 = y0 + h;
		if (x0 > x1) swap(x0, x1);
		if (y0 > y1) swap(y0, y1);
		limit(x0, 0, nWidth - 1);
		limit(y0, 0, nHeight - 1);
		limit(x1, 0, nWidth - 1);
		limit(y1, 0, nHeight - 1);

		byte b = (nColor & 0x000000ff);
		byte g = (nColor & 0x0000ff00) >> 8;
		byte r = (nColor & 0x00ff0000) >> 16;
		const int nLineSize = WidthBytes(nBitCount*nWidth);
		const int nPixelSize = nBitCount / 8;
		//往内部收缩
		for (int i = 0; i < nPenSize; i++)
		{
			byte* p0 = (byte*)pData + min(y0 + i, y1) * nLineSize + x0 * nPixelSize;
			byte* p1 = (byte*)pData + max(y1 - i, y0) * nLineSize + x0 * nPixelSize;

			for (int x = x0; x <= x1; x++)
			{
				*(p0 + 0) = b;
				*(p0 + 1) = g;
				*(p0 + 2) = r;

				*(p1 + 0) = b;
				*(p1 + 1) = g;
				*(p1 + 2) = r;

				p0 += nPixelSize;
				p1 += nPixelSize;
			}

			p0 = (byte*)pData + y0 * nLineSize + min(x0 + i, x1) * nPixelSize;
			p1 = (byte*)pData + y0 * nLineSize + max(x1 - i, x0) * nPixelSize;

			for (int y = y0; y <= y1; y++)
			{
				*(p0 + 0) = b;
				*(p0 + 1) = g;
				*(p0 + 2) = r;

				*(p1 + 0) = b;
				*(p1 + 1) = g;
				*(p1 + 2) = r;

				p0 += nLineSize;
				p1 += nLineSize;
			}
		}
	}	

}
