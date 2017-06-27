//
// Created by yongfali on 2016/11/10.
//

#include "include/GPUImageWriteFilter.h"

namespace e
{
	shader_t kGPUImageWrite2FragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;
		varying highp vec2 textureCoordinate;

		void main()
		{
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate).bgra;
		}
	);

	GPUImageWriteFilter::GPUImageWriteFilter()
	{
		_outSize.width = DEFAULT_OUTPUT_WIDTH;
		_outSize.height = DEFAULT_OUTPUT_HEIGHT;
		_outBuffer = 0;
		_callback = 0;

		memset(&_vs, 0, sizeof(_vs));

		_vs.size = _outSize.Area() * 4;
		_vs.data = (char*)malloc(_vs.size);
		_vs.width = _outSize.width;
		_vs.height = _outSize.height;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageWriteFilter::~GPUImageWriteFilter()
	{
		SafeFree(&_vs.data);
	}

	void GPUImageWriteFilter::SetOutputSize(Size size)
	{
		if (_outSize != size && size.Area()>0)
		{
			_outSize = size;
			_vs.size = size.Area() * 4;
			_vs.data = (char*)realloc(_vs.data, _vs.size);
			_vs.width = size.width;
			_vs.height = size.height;
			_vs.format = 0;//KG_BGRA;
		}
	}

	void GPUImageWriteFilter::SetOutputCallback(GPUImageCallback callback)
	{
		_callback = callback;
	}

	void GPUImageWriteFilter::SetOutputCallback(char *buffer)
	{
		_outBuffer = buffer;
	}

	bool GPUImageWriteFilter::Initialize()
	{
		if (!GPUImageFilter::Initialize(kGPUImageVertexShaderString, kGPUImageWrite2FragmentShaderString))
		{
			LOGE("GPUImageWriteFilter initialize failed");
			return false;
		}

		LOGV("GPUImageWriteFilter initialize ok");
		return true;
	}

	Size GPUImageWriteFilter::GetSizeOfFBO() const
	{
		return _outSize;
	}

	void GPUImageWriteFilter::RenderToTexture(const GLfloat *vertices, const GLfloat *texCoords)
	{
		//OpenGL默认Y坐标是正向向上，屏幕显示时Y坐标是正向向下
		const GLfloat texCoords2[8] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f
		};

		if (_outBuffer || _callback)
		{
			GPUImageFilter::RenderToTexture(vertices, texCoords2);
			//read data from GPU memory
			glBindTexture(GL_TEXTURE_2D, _outputFramebuffer->GetTexture());
			glReadPixels(0, 0, _vs.width, _vs.height, GL_RGBA, GL_UNSIGNED_BYTE, _outBuffer?_outBuffer:_vs.data);

			if (!_outBuffer && _callback)
			{
				_callback(&_vs);
			}
		}
		else
		{
			_firstInputFramebuffer = NULL;
		}
	}
}
