//
// Created by yongfali on 2016/11/3.
//

#ifndef E_GPUIMAGE_PINKBLURFILTER2_H
#define E_GPUIMAGE_PINKBLURFILTER2_H

#include "GPUImageGaussianBlurFilter.h"

namespace e
{
	//单通道高斯模糊，适用于性能差的设备
	class GPUImagePinkBlurFilter2
		: public GPUImageGaussianBlurFilter
	{
	public:
		DEFINE_CREATE(GPUImagePinkBlurFilter2)
	CONSTRUCTOR_ACCES:
		GPUImagePinkBlurFilter2();
		virtual ~GPUImagePinkBlurFilter2();
	public:
		virtual bool Initialize();
		virtual string CreateFragmentShaderString(int blurRadius, float sigma);
	};

    typedef Ptr<GPUImagePinkBlurFilter2> GPUImagePinkBlurFilter2Ptr;
}

#endif //KUGOUEFFECT_GPUIMAGEPINKBLURFILTER2_H
