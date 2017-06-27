//
// Created by yongfali on 2016/11/7.
//

#include "include/GPUImageScaleFilter.h"

namespace e
{
	shader_t kGPUImageScaleVertexShaderString = SHADER_STRING
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

	shader_t kGPUImageScaleFragmentShaderString = SHADER_STRING
	(
		varying highp vec2 textureCoordinate;
		uniform sampler2D inputImageTexture;

		void main()
		{
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
		}
	);

	GPUImageScaleFilter::GPUImageScaleFilter()
	{
		_tmpSize.width = 540;
		_tmpSize.height = 960;
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageScaleFilter::~GPUImageScaleFilter()
	{

	}

	bool GPUImageScaleFilter::Initialize()
	{
		return GPUImageTwoPassFilter::Initialize(kGPUImageScaleVertexShaderString
			, kGPUImageScaleFragmentShaderString
			, kGPUImageScaleVertexShaderString
			, kGPUImageScaleFragmentShaderString);
	}

	void GPUImageScaleFilter::SetOutputSize(const Size size)
	{
		_outSize = size;
		_tmpSize.width = 540;
		_tmpSize.height = 960;
	}

	void GPUImageScaleFilter::SetOutputRect(const Rect rect)
	{
		_outRect = rect;
	}

	void GPUImageScaleFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
	{
		//do scale & crop
		if (_outSize != GetSizeOfFBO())
		{
			_filterProgram->Use();
			_outputFramebuffer = FetchFramebuffer(_tmpSize);
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

			//second rendering
			_filterProgram2->Use();
			_outputFramebuffer2 = FetchFramebuffer(_outSize);
			_outputFramebuffer2->Active();

			SetProgramUniforms(1);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, _outputFramebuffer->GetTexture());
			glVertexAttribPointer(_filterTextureCoordinateAttribute2, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

			glUniform1i(_filterInputTextureUniform2, 3);
			glVertexAttribPointer(_filterPositionAttribute2, 2, GL_FLOAT, GL_FALSE, 0, vertices);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			_outputFramebuffer = NULL;
		}
		else
		{
			_outputFramebuffer2 = _firstInputFramebuffer;
		}
	}
}
