//
// Created by yongfali on 2016/3/25.
//

#include "include/GPUImageFilter.h"

namespace e 
{
	shader_t kGPUImageVertexShaderString = SHADER_STRING 
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;
		varying vec2 textureCoordinate;

		void main()
		{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;
		}
	);

	shader_t kGPUImagePassthroughFragmentShaderString = SHADER_STRING
	(
		varying highp vec2 textureCoordinate;
		uniform sampler2D inputImageTexture;

		void main()
		{
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
		}
	);

	GPUImageFilter::GPUImageFilter(void)
	{
		_filterName = "GPUImageFilter";
		_enable = true;
		_filterPositionAttribute = 0;
		_filterTextureCoordinateAttribute = 0;
		_filterInputTextureUniform = -1;
		_bgColorRed = 0.0f;
		_bgColorGreen = 0.0f;
		_bgColorBlue = 0.0f;
		_bgColorAlpha = 1.0f;
		_inputRotation = kGPUImageNoRotation;
		_frameBufferCache = GPUImageFramebufferCache::Singleton();
	}

	GPUImageFilter::~GPUImageFilter(void)
	{
		_filterProgram = 0;
		_firstInputFramebuffer = 0;
		_frameBufferCache = 0;
	}

	void GPUImageFilter::SetInputSize(Size size, int index)
	{
		_inputTextureSize = size;

		SetupFilter(GetSizeOfFBO());

		//LOGD("% set input size", _filterName.c_str());
	}

	void GPUImageFilter::SetInputRotationMode(GPUImageRotationMode & rotateMode, int index)
	{
		_inputRotation = rotateMode;
	}

	void GPUImageFilter::SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index)
	{
		_firstInputFramebuffer = frameBuffer;
	}

	void GPUImageFilter::SetInputTextureOptions(GPUImageTextureOptions & textureOptions, int index)
	{
		_inputTextureOptions = textureOptions;
	}

	void GPUImageFilter::EndProcessing(void)
	{

	}

	bool GPUImageFilter::IsEnable(void)
	{
		return _enable;
	}

	int GPUImageFilter::NextAvailableTextureIndex(void)
	{
		return 0;
	}

	Size GPUImageFilter::GetSizeOfFBO(void) const 
	{
        Size size = _inputTextureSize;
        if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
        {
            size.width = _inputTextureSize.height;
            size.height = _inputTextureSize.width;
        }
		return size;
	}

	Size GPUImageFilter::GetOutputFrameSize(void) const 
	{
		return GetSizeOfFBO();
	}

	GLProgramPtr GPUImageFilter::GetFilterProgram(void)
	{
		return _filterProgram;
	}

	GPUImageFramebufferPtr GPUImageFilter::GetFirstInputFramebuffer(void)
	{
		return _firstInputFramebuffer;
	}

	GPUImageTextureOptions GPUImageFilter::GetInputTextureOptions(void) const
	{
		return _inputTextureOptions;
	}

	GPUImageTextureOptions GPUImageFilter::GetOutputTextureOptions(void) const
	{
		return _outputTextureOptions;
	}

	Size GPUImageFilter::GetSizeOfRotated(Size rotateSize, int index)
	{
		Size rotatedSize = rotateSize;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			rotatedSize.width = rotateSize.height;
			rotatedSize.height = rotateSize.width;
		}
		return rotatedSize;
	}

	void GPUImageFilter::InitializeAttributes(void)
	{
		_filterProgram->AddAttribute("position");
		_filterProgram->AddAttribute("inputTextureCoordinate");
	}

	void GPUImageFilter::SetProgramUniforms(int index)
	{
        //TODO : you can ovrride to provide some custom setup
	}

	void GPUImageFilter::SetupFilter(Size size)
	{
		//TODO : you can ovrride to provide some custom setup
	}

	bool GPUImageFilter::Initialize(void)
	{
		return Initialize(kGPUImageVertexShaderString, kGPUImagePassthroughFragmentShaderString);
	}

	bool GPUImageFilter::Initialize(const char* vShaderString, const char* fShaderString)
	{
		_filterProgram = new GLProgram(vShaderString, fShaderString);
		CheckPointer(_filterProgram, false);
		
		if (!_filterProgram->IsValid())
		{
			InitializeAttributes();

			if (!_filterProgram->Link())
			{
				std::string log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_PROG);
				LOGE("opengl shader program link failed:prog %s\n", log.c_str());
				log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_VERT);
				LOGE("opengl shader program link failed:vert %s\n", log.c_str());
				log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_FRAG);
				LOGE("opengl shader program link failed:frag %s\n", log.c_str());
				_filterProgram = NULL;
				return false;
			}
		}

		_filterProgram->Use();
		_filterPositionAttribute = _filterProgram->GetAttributeIndex("position");
		_filterTextureCoordinateAttribute = _filterProgram->GetAttributeIndex("inputTextureCoordinate");
		_filterInputTextureUniform = _filterProgram->GetUniformLocation("inputImageTexture");
		
		glEnableVertexAttribArray(_filterPositionAttribute);
		glEnableVertexAttribArray(_filterTextureCoordinateAttribute);

		LOGV("GPUImageFilter initialize ok:%d,%d", _filterPositionAttribute, _filterTextureCoordinateAttribute);
		return true;
	}

	bool GPUImageFilter::Initialize(const char* fShaderString)
	{
		return Initialize(kGPUImageVertexShaderString, fShaderString);
	}
	
	GPUImageFramebufferPtr GPUImageFilter::FetchFramebuffer(const Size size)
	{
		return _frameBufferCache->FetchFramebuffer(size, _outputTextureOptions, false);
	}

	void GPUImageFilter::NewFrameReady(Time frameTime, int index)
	{
		//没有旋转，输入是什么就是什么
		const GLfloat imageVertices[] = {
			-1.0f, -1.0f, 
			 1.0f, -1.0f,
			-1.0f,  1.0f,
			 1.0f,  1.0f
		};

		const GLfloat texCoordinates[] = {
			0.0f, 0.0f,
			1.0f, 0.0f, 
			0.0f, 1.0f, 
			1.0f, 1.0f
		};

	    RenderToTexture(imageVertices, texCoordinates);

	    CallNextTargets(frameTime);
	}	

	void GPUImageFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
	{
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		SetProgramUniforms(0);

		glClearColor(_bgColorRed, _bgColorGreen, _bgColorBlue, _bgColorAlpha);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
	}

	void GPUImageFilter::CallNextTargets(Time frameTime)
	{
		TargetList::iterator it = _targets->begin();
		for (; it != _targets->end(); it++)
		{
			GPUImageInput* target = it->target;
			if (target->IsEnable())
			{
                //旋转参数不传给下一个target
				target->SetInputSize(GetOutputFrameSize(), it->index);
				target->SetInputFramebuffer(GetOutputFramebuffer(), it->index);
			}
		}

        RemoveOutputFramebuffer();
		
		//call next target render
		for(it = _targets->begin(); it != _targets->end(); it++)
		{
			GPUImageInput* target = it->target;
			if (target->IsEnable())
			{
				target->NewFrameReady(frameTime, it->index);
			}
		}
	}

	void GPUImageFilter::Reset()
	{
		_inputRotation = kGPUImageNoRotation;
		_inputTextureSize = kSizeZero;
	}
}//end namespace
