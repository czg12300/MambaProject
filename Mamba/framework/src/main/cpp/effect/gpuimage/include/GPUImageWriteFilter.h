//
// Created by yongfali on 2016/11/10.
//

#ifndef E_GPUImageWriteFilter_H
#define E_GPUImageWriteFilter_H

#include "GPUImageFilter.h"
#include "GPUImageCallback.h"

namespace e
{
	struct VideoSample
	{
		char* data;
		int size;
		int width;
		int height;
		int format;
	};

	//读取处理数据类
	class GPUImageWriteFilter: public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageWriteFilter)
	CONSTRUCTOR_ACCES:
		GPUImageWriteFilter();
		~GPUImageWriteFilter();
	public:
		void SetOutputSize(Size size);
		void SetOutputCallback(GPUImageCallback callback);
		void SetOutputCallback(char* buffer);
		virtual bool Initialize(void);
		virtual Size GetSizeOfFBO(void) const;
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
	private:
		Size _outSize;
		char* _outBuffer;
		VideoSample _vs;
		GPUImageCallback _callback;
	};
}

#endif //E_GPUImageWriteFilter_H
