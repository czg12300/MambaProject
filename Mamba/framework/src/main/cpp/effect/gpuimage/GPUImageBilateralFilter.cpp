#include "include/GPUImageBilateralFilter.h"
#include "Log.h"

namespace e
{
	shader_t kGPUImageBilateralVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;

		const int GAUSSIAN_SAMPLES = 9;
		varying vec2 textureCoordinate;
		varying vec2 blurCoordinates[GAUSSIAN_SAMPLES];

		uniform float texelWidthOffset;
		uniform float texelHeightOffset;

		void main()
		{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;

			// Calculate the positions for the blur
			int multiplier = 0;
			highp vec2 blurStep;
			highp vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);

			for (int i = 0; i < GAUSSIAN_SAMPLES; i++)
			{
				multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));
				// Blur in x (horizontal)
				blurStep = float(multiplier) * singleStepOffset;
				blurCoordinates[i] = inputTextureCoordinate.xy + blurStep;
			}
		}
	);

	shader_t kGPUImageBilateralFragmentShaderString = SHADER_STRING
	(
		precision highp float;

		const int GAUSSIAN_SAMPLES = 9;
		varying highp vec2 textureCoordinate;
		varying highp vec2 blurCoordinates[GAUSSIAN_SAMPLES];

		uniform sampler2D inputImageTexture;
		uniform float distanceFactor;

		void main()
		{
			lowp float centralColor;
			lowp float sampleColor;
			lowp float sumColor;
			lowp float distance;
			lowp float weight;
			lowp float sumWeight;

			centralColor = texture2D(inputImageTexture, blurCoordinates[4]).g;
			sumWeight = 0.18;
			sumColor = centralColor * 0.18;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[0]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.05 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[1]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.09 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[2]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.12 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[3]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.15 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[5]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.15 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[6]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.12 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[7]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.09 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;

			sampleColor = texture2D(inputImageTexture, blurCoordinates[8]).g;
			distance = min(distance(centralColor, sampleColor) * distanceFactor, 1.0);
			weight = 0.05 * (1.0 - distance);
			sumWeight += weight;
			sumColor += sampleColor * weight;
			sumColor /= sumWeight;

			gl_FragColor = vec4(sumColor, sumColor, sumColor, 1.0);
		}
	);

	GPUImageBilateralFilter::GPUImageBilateralFilter(void)
	{
		firstDistanceFactorUniform = -1;
		secondDistanceFactorUniform = -1;

		distanceFactor = 3.0f;
		SetTexelSpacingMultiplier(1.0f);

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageBilateralFilter::~GPUImageBilateralFilter(void)
	{
	
	}

	bool GPUImageBilateralFilter::Initialize(void)
	{
		if (!GPUImageTwoPassTextureSamplingFilter::Initialize(kGPUImageBilateralVertexShaderString
			, kGPUImageBilateralFragmentShaderString
			, kGPUImageBilateralVertexShaderString
			, kGPUImageBilateralFragmentShaderString))
		{
			LOGE("GPUImageBilateralFilter Initialize failed!");
			return false;
		}

		firstDistanceFactorUniform = _filterProgram->GetUniformLocation("distanceFactor");
		secondDistanceFactorUniform = _filterProgram2->GetUniformLocation("distanceFactor");

		LOGD("GPUImageBilateralFilter initialize ok: %d,%d"
			, firstDistanceFactorUniform
			, secondDistanceFactorUniform);

		return true;
	}

	void GPUImageBilateralFilter::SetDistanceFactor(float factor)
	{
		distanceFactor = factor;
		LOGD("GPUImageBilateralFilter::SetDistanceFactor %f", factor);
	}

	void GPUImageBilateralFilter::SetTexelSpacingMultiplier(float multiplier)
	{
		texelSpacingMultiplier = multiplier;
		SetVerticalTexelSpacing(multiplier);
		SetHorizontalTexelSpacing(multiplier);
	}

	void GPUImageBilateralFilter::SetProgramUniforms(int index)
	{
		GPUImageTwoPassTextureSamplingFilter::SetProgramUniforms(index);

		if (index == 0) {
			//glUniform1f(firstDistanceFactorUniform, distanceFactor);
			_filterProgram->SetUniform1f("distanceFactor", distanceFactor);
		}else{
			//glUniform1f(secondDistanceFactorUniform, distanceFactor);
			_filterProgram2->SetUniform1f("distanceFactor", distanceFactor);
		}
	}    
}