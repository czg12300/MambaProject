//
// Created by yongfali on 2016/11/14.
//

#include "include/GPUImageSharpenFilter2.h"

namespace e
{
	shader_t kGPUImageSharpen2VertexShaderString = SHADER_STRING
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

	shader_t kGPUImageSharpen2FragmentShaderString = SHADER_STRING
	(
		precision highp float;
		uniform sampler2D inputImageTexture;
		uniform sampler2D inputImageTexture2;

		varying highp vec2 textureCoordinate;
		varying highp vec2 textureCoordinate2;

		uniform float alphaFactor;

		void main()
		{
			lowp vec4 a = texture2D(inputImageTexture, textureCoordinate);
			lowp vec4 b = texture2D(inputImageTexture2, textureCoordinate2);
			lowp vec4 c = b + 2.0 * a - 1.0;
			gl_FragColor = mix(b, c, alphaFactor);
		}
	);

	GPUImageSharpenFilter2::GPUImageSharpenFilter2()
	{
		alphaFactor = 0.75f;
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageSharpenFilter2::~GPUImageSharpenFilter2()
	{

	}

	bool GPUImageSharpenFilter2::Initialize()
	{
		if (!GPUImageTwoInputFilter::Initialize(kGPUImageSharpen2VertexShaderString
			, kGPUImageSharpen2FragmentShaderString))
		{
			LOGE("GPUImageSharpenFilter2 initialize failed");
			return false;
		}

		//alphaFactorUniform = _filterProgram->GetUniformLocation("alphaFactor");
		LOGV("GPUImageSharpenFilter2 initialize ok");
		return true;
	}

	void GPUImageSharpenFilter2::SetProgramUniforms(const int index)
	{
		if (_filterProgram)
		{
			_filterProgram->SetUniform1f("alphaFactor", alphaFactor);
		}
	}

	void GPUImageSharpenFilter2::SetSharpness(float value)
	{
		alphaFactor = max(0.0f, min(value, 1.0f));
	}
}
