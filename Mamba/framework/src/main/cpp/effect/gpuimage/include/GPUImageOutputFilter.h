//
// Created by yongfali on 2016/4/11.
//

#ifndef E_GPUIMAGE_SCALEFILTER_H
#define E_GPUIMAGE_SCALEFILTER_H

#include "GPUImageBase.h"
#include "GPUImageFilter.h"
#include "GPUImageCallback.h"
#include "IFaceTracker.h"

namespace e 
{
	extern shader_t kGPUImageSwapChannelFragmentShaderString;
	
	class GPUImageOutputFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageOutputFilter)
	CONSTRUCTOR_ACCES:
		GPUImageOutputFilter(void);
		virtual ~GPUImageOutputFilter(void);
	public:
		void SetOutputSize(Size size);
		void SetWriteEnable(bool enable);
		void SetSampleCallback(GPUImageCallback callback);

		virtual bool Initialize(void);
		virtual Size GetSizeOfFBO(void) const;
	protected:
		virtual void SetProgramUniforms(int index);
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
	protected:
		char* _buffer;
		Size _outputSize;
		bool _enableWrite;
		ImageSample _imageSample;
		GPUImageCallback _sampleCallback;
	};

	typedef Ptr<GPUImageOutputFilter> GPUImageScaleFilterPtr;
}

#endif //E_GPUIMAGE_SCALEFILTER_H
