//
// Created by yongfali on 2016/4/11.
//

#include "include/GPUImageOutputFilter.h"

namespace e
{
	shader_t kGPUImageSwapChannelFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;
		varying highp vec2 textureCoordinate;

		void main()
		{
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate).bgra;
		}
	);

	GPUImageOutputFilter::GPUImageOutputFilter(void)
	{
		_enableWrite = false;
		_outputSize.width = 360;
		_outputSize.height = 640;
		memset(&_imageSample, 0, sizeof(ImageSample));
		_buffer = (char*)malloc(_outputSize.Area() * 4);
		assert(_buffer);
		
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif	
	}

	GPUImageOutputFilter::~GPUImageOutputFilter(void)
	{
		SafeFree(&_buffer);
	}

	bool GPUImageOutputFilter::Initialize(void)
	{
		if (!GPUImageFilter::Initialize(kGPUImageVertexShaderString
				, kGPUImageSwapChannelFragmentShaderString))
		{
			LOGE("GPUImageOutputFilter initialize failed!");
			return false;
		}
		
		LOGD("GPUImageOutputFilter initialize ok");
		return true;
	}

	void GPUImageOutputFilter::SetOutputSize(Size size)
	{
		if (_outputSize != size && size != kSizeZero)
		{
			_outputSize = size;
			_buffer = (char*)realloc(_buffer, _outputSize.Area()*4);
			if (!_buffer){
				LOGE("GPUImageOutputFilter::SetOutputSize alloc buffer failed");
			}else{
				LOGV("GPUImageOutputFilter::SetOutputSize %d x %d", _outputSize.width, _outputSize.height);
			}
		}
	}

	void GPUImageOutputFilter::SetSampleCallback(GPUImageCallback callback)
	{
		_sampleCallback = callback;
	}

	void GPUImageOutputFilter::SetWriteEnable(bool enableWrite)
	{
		_enableWrite = enableWrite;
	}

	Size GPUImageOutputFilter::GetSizeOfFBO(void) const
	{
		return _outputSize;	
	}

	void GPUImageOutputFilter::SetProgramUniforms(int index)
	{
		GPUImageFilter::SetProgramUniforms(index);
	}

	void GPUImageOutputFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
	{
		//OpenGL默认Y坐标是正向向上，屏幕显示时Y坐标是正向向下	
		const GLfloat texCoords2[8] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f
		};

		//此处进行人脸识别处理
		if (_sampleCallback)
		{
			GPUImageFilter::RenderToTexture(vertices, texCoords2);
			glBindTexture(GL_TEXTURE_2D, _outputFramebuffer->GetTexture());
			glReadPixels(0, 0, _outputSize.width, _outputSize.height, GL_RGBA, GL_UNSIGNED_BYTE, _buffer);
		
			_imageSample.data = _buffer;
			_imageSample.size = _outputSize.Area() * 4;
			_imageSample.width = _outputSize.width;
			_imageSample.height = _outputSize.height;
			_imageSample.bitCount = 32;
			_imageSample.format = GL_RGBA;
			//回调给人脸识别模块处理
			_sampleCallback(&_imageSample);	
		}

	//for debug
#ifdef  _DEBUG
		static int fileCount = 0;
		if (_enableWrite && (++fileCount%25==0))
		{
			int fileIndex = fileCount / 25;
			int bufferSize = _outputSize.Area() * 4;
			char fileName[256] = {0};
			sprintf(fileName, "%st%03d.bmp", GPUImageEnvironment::GetPath(KG_PATH_OUTPUT).c_str(), fileIndex);
			Bitmap::Save(fileName, _outputSize.width, _outputSize.height, 32, _buffer, bufferSize);
			LOGV("GPUImageOutputFilter save bmp: %d x %d, %s", _outputSize.width, _outputSize.height, fileName);
		}
#endif
	}
}
