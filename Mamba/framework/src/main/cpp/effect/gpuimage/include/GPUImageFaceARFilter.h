//
// Created by yongfali on 2016/4/13.
//

#ifndef E_GPUIMAGE_FACEUFILTER_H
#define E_GPUIMAGE_FACEUFILTER_H
#include "GPUImageTwoInputFilter.h"

namespace e 
{
	extern shader_t kGPUImageFaceARVertexShaderString;
	extern shader_t kGPUImageFaceARFragmentShaderString;

	class GPUImageFaceARFilter: public GPUImageTwoInputFilter
	{
	public:
		DEFINE_CREATE(GPUImageFaceARFilter)
	CONSTRUCTOR_ACCES:
		GPUImageFaceARFilter(void);
		virtual ~GPUImageFaceARFilter(void);
	public:
		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString, const char* fShaderString);
		//override render
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
		virtual Size GetSizeOfFBO(void) const;
	};

	typedef Ptr<GPUImageFaceARFilter> GPUImageFaceARFilterPtr;
}

#endif //_E_GPUIMAGE_FACEUFILTER_H_
