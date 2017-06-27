//
// Created by yongfali on 2016/4/9.
//

#include "include/GPUImageDisplayFilter.h"

namespace e
{
	enum {
		ATTR_INDEX_VERTEX = 0,
		ATTR_INDEX_TEXCOORD = 1,
	};

	// const int ATTR_INDEX_VERTEX = 0;
	// const int ATTR_INDEX_TEXCOORD = 1;

	shader_t kGPUImageDisplayVertexShaderString = SHADER_STRING
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

	shader_t kGPUImageDisplayFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;
		varying highp vec2 textureCoordinate;

		void main() 
		{
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
		}
	);

	GPUImageDisplayFilter::GPUImageDisplayFilter(void)
	{
		_vp.x = 0;
		_vp.y = 0;
		_vp.width = 360;
		_vp.height = 640;
		_inputRotation = kGPUImageNoRotation;
		_filterPositionAttribute = 0;
		_filterTextureCoordinateAttribute = 0;
		_filterInputTextureUniform = -1;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageDisplayFilter::~GPUImageDisplayFilter(void)
	{

	}

	bool GPUImageDisplayFilter::Initialize(void)
	{
		return Initialize(kGPUImageDisplayVertexShaderString, kGPUImageDisplayFragmentShaderString);
	}

	bool GPUImageDisplayFilter::Initialize(const char* vShaderString, const char* fShaderString)
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

			_filterPositionAttribute = _filterProgram->GetAttributeIndex("position");
		  	_filterTextureCoordinateAttribute = _filterProgram->GetAttributeIndex("inputTextureCoordinate");
		  	_filterInputTextureUniform = _filterProgram->GetUniformLocation("inputImageTexture");
		}

		_filterProgram->Use();
		glEnableVertexAttribArray(_filterPositionAttribute);
		glEnableVertexAttribArray(_filterTextureCoordinateAttribute);
		LOGV("GPUImageDisplayFilter initialize ok: %d,%d,%d"
			, _filterPositionAttribute
			, _filterTextureCoordinateAttribute
			, _filterInputTextureUniform);
		return true;
	}

	void GPUImageDisplayFilter::InitializeAttributes(void) 
	{
	    _filterProgram->AddAttribute("position");
	    _filterProgram->AddAttribute("inputTextureCoordinate");
	}

	void GPUImageDisplayFilter::SetViewport(int x, int y, int width, int height)
	{
		_vp.x = x;
		_vp.y = y;
		_vp.width = width;
		_vp.height = height;
		LOGD("GPUImageDisplayFilter set viewport %d,%d,%d,%d",x,y,width,height);
	}

	void GPUImageDisplayFilter::SetInputSize(Size size, int textureIndex)
	{
		_inputTextureSize = size;
	}

	void GPUImageDisplayFilter::SetInputRotationMode(GPUImageRotationMode & rotation, int index)
	{
		_inputRotation = rotation;
	}

	void GPUImageDisplayFilter::SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index)
	{
		_inputFramebuffer = frameBuffer;
	}

	int GPUImageDisplayFilter::NextAvailableTextureIndex(void)
	{
		return 0;
	}

	void GPUImageDisplayFilter::NewFrameReady(Time time, int index)
	{	
		//OpenGL默认Y坐标是正向向上，屏幕显示时Y坐标是正向向下
		const GLfloat squareVertices[] = {
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			-1.0f,  1.0f,
			 1.0f,  1.0f
		};

	    const GLfloat textureCoordinates[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
	    };

		_filterProgram->Use();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(_vp.x, _vp.y, _vp.width, _vp.height);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _inputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glVertexAttribPointer(ATTR_INDEX_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, squareVertices);
		glVertexAttribPointer(ATTR_INDEX_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinates);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_inputFramebuffer = NULL;
	}

	void GPUImageDisplayFilter::EndProcessing(void)
	{

	}

	bool GPUImageDisplayFilter::IsEnable(void)
	{
		return true;
	}

	void GPUImageDisplayFilter::Reset(void)
	{
		
	}
}