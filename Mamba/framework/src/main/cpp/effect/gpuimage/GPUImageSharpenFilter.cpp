#include "include/GPUImageSharpenFilter.h"

namespace e
{
	shader_t kGPUImageSharpenVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;

		varying vec2 textureCoordinate;
		varying vec2 leftTextureCoordinate;
		varying vec2 rightTextureCoordinate;
		varying vec2 topTextureCoordinate;
		varying vec2 bottomTextureCoordinate;
		varying float centerMultiplier;
		varying float edgeMultiplier;

		uniform float imageWidthOffset;
		uniform float imageHeightOffset;
		uniform float sharpness;

		void main()
		{
			gl_Position = position;

			highp vec2 widthStep = vec2(imageWidthOffset, 0.0);
			highp vec2 heightStep = vec2(0.0, imageHeightOffset);

			textureCoordinate = inputTextureCoordinate.xy;
			leftTextureCoordinate = inputTextureCoordinate.xy - widthStep;
			rightTextureCoordinate = inputTextureCoordinate.xy + widthStep;
			topTextureCoordinate = inputTextureCoordinate.xy + heightStep;
			bottomTextureCoordinate = inputTextureCoordinate.xy - heightStep;

			centerMultiplier = 1.0 + 4.0 * sharpness;
			edgeMultiplier = sharpness;
		}
	);

	shader_t kGPUImageSharpenFragmentShaderString = SHADER_STRING
	(
		varying highp vec2 textureCoordinate;
		varying highp vec2 leftTextureCoordinate;
		varying highp vec2 rightTextureCoordinate;
		varying highp vec2 topTextureCoordinate;
		varying highp vec2 bottomTextureCoordinate;
		varying highp float centerMultiplier;
		varying highp float edgeMultiplier;

		uniform sampler2D inputImageTexture;

		void main()
		{
			lowp vec3 textureColor = texture2D(inputImageTexture, textureCoordinate).rgb;
			lowp vec3 leftTextureColor = texture2D(inputImageTexture, leftTextureCoordinate).rgb;
			lowp vec3 rightTextureColor = texture2D(inputImageTexture, rightTextureCoordinate).rgb;
			lowp vec3 topTextureColor = texture2D(inputImageTexture, topTextureCoordinate).rgb;
			lowp vec3 bottomTextureColor = texture2D(inputImageTexture, bottomTextureCoordinate).rgb;
			lowp vec4 color = vec4((textureColor * centerMultiplier - (leftTextureColor * edgeMultiplier + rightTextureColor * edgeMultiplier + topTextureColor * edgeMultiplier + bottomTextureColor * edgeMultiplier)), texture2D(inputImageTexture, bottomTextureCoordinate).w);
			gl_FragColor = vec4(mix(textureColor, color.rgb, 0.5), 1.0);
		}
	);

	GPUImageSharpenFilter::GPUImageSharpenFilter(void)
	{
		_imageWidthOffset = 0.0f;
		_imageHeightOffset = 0.0f;
		_sharpness = 0.5f;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageSharpenFilter::~GPUImageSharpenFilter(void)
	{

	}

	bool GPUImageSharpenFilter::Initialize(void)
	{
		if (!GPUImageFilter::Initialize(kGPUImageSharpenVertexShaderString, kGPUImageSharpenFragmentShaderString))
		{
			LOGE("GPUImageSharpenFilter Initialize failed");
			return false;
		}

		return true;
	}

	void GPUImageSharpenFilter::RenderToTexture(const float* vertices, const float* texCoords)
	{
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		SetProgramUniforms(0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
	}

	void GPUImageSharpenFilter::SetupFilter(Size size)
	{
		GPUImageFilter::SetupFilter(size);

		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			_imageWidthOffset = 1.0f / size.height;
			_imageHeightOffset = 1.0f / size.width;
		}else{
			_imageWidthOffset = 1.0f / size.width;
			_imageHeightOffset = 1.0f / size.height;
		}
	}

	void GPUImageSharpenFilter::SetSharpness(float sharpness)
	{
		_sharpness = max(0.0f, min(sharpness, 1.0f));
	}

	void GPUImageSharpenFilter::SetProgramUniforms(int index)
	{
		GPUImageFilter::SetProgramUniforms(index);

		if (_filterProgram)
		{
			_filterProgram->SetUniform1f("sharpness", _sharpness);
			_filterProgram->SetUniform1f("imageWidthOffset", _imageWidthOffset);
			_filterProgram->SetUniform1f("imageHeightOffset", _imageHeightOffset);
		}
	}
}