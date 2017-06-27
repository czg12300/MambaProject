//
// Created by yongfali on 2016/4/13.
//

#include "include/GPUImageFaceARFilter.h"

namespace e
{
	shader_t kGPUImageFaceARVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;
		attribute vec4 inputTextureCoordinate2;

		varying vec2 textureCoordinate;
		varying vec2 textureCoordinate2;

		void main() 
		{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;
			textureCoordinate2 = inputTextureCoordinate2.xy;
		}
	);

	shader_t kGPUImageFaceARFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;
		uniform sampler2D inputImageTexture2;
		varying highp vec2 textureCoordinate;
		varying highp vec2 textureCoordinate2;

		void main()
        {
			lowp vec4 source = texture2D(inputImageTexture, textureCoordinate);
            lowp vec4 effect = texture2D(inputImageTexture2, textureCoordinate2);
			gl_FragColor = mix(source, effect, effect.a);
		}
	);

	GPUImageFaceARFilter::GPUImageFaceARFilter(void)
	{
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageFaceARFilter::~GPUImageFaceARFilter(void)
	{

	}

	bool GPUImageFaceARFilter::Initialize(void)
	{
		return Initialize(kGPUImageFaceARVertexShaderString, kGPUImageFaceARFragmentShaderString);
	}

	bool GPUImageFaceARFilter::Initialize(const char* vShaderString, const char* fShaderString)
	{
		if (!GPUImageTwoInputFilter::Initialize(vShaderString, fShaderString))
		{
			LOGE("GPUImageFaceARFilter initialize failed!");
			return false;
		}

		LOGD("GPUImageFaceARFilter initialize ok");
		return true;
	}

	Size GPUImageFaceARFilter::GetSizeOfFBO(void) const 
	{
		Size filterSize = _inputTextureSize;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			filterSize.width = _inputTextureSize.height;
			filterSize.height = _inputTextureSize.width;
		}
		return filterSize;
	}

	void GPUImageFaceARFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
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

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _secondInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform2, 3);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0,texCoords);
		glVertexAttribPointer(_filterSecondTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
		_secondInputFramebuffer = NULL;
	}
}
