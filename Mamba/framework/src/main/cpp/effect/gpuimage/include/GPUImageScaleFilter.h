//
// Created by yongfali on 2016/11/7.
//

#ifndef E_GPUIMAGE_SCALEFILTER_H
#define E_GPUIMAGE_SCALEFILTER_H

#include "GPUImageTwoPassFilter.h"

namespace e
{
	class GPUImageScaleFilter : public GPUImageTwoPassFilter
	{
	public:
		DEFINE_CREATE(GPUImageScaleFilter)
	CONSTRUCTOR_ACCES:
		GPUImageScaleFilter();
		~GPUImageScaleFilter();
	public:
		bool Initialize();
		void SetOutputSize(const Size size);
		void SetOutputRect(const Rect rect);
		void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
	protected:
		Size _outSize;
		Rect _outRect;
		Size _tmpSize;
		bool _isNeedScale;
	};

    typedef Ptr<GPUImageScaleFilter> GPUImageScaleFilterPtr;
}

#endif //E_GPUIMAGE_SCALEFILTER_H
