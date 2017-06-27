//
// Created by yongfali on 2016/3/24.
//

#ifndef __E_BITMAP_H__
#define __E_BITMAP_H__

namespace e
{
    class I420sp{
    public:
    		static bool Save(const char* fileName, int width, int height, void* bits, int size);
    };

    class Bitmap {
    public:
        static bool Save(const char* fileName, int width, int height, int bitCount, void* bits, int size);
    };
    
    void DrawPoint(int x
        , int y
        , int nPenSize
        , int nColor
        , void* pData
        , int nSize
        , int nWidth
        , int nHeight
        , int nBitCount);

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
        , int nBitCount);
        
}

#endif //__E_BITMAP_H__
