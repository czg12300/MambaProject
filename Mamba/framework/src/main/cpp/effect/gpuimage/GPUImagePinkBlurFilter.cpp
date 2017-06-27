//
// Created by yongfali on 2016/10/28.
//

#include "include/GPUImagePinkBlurFilter.h"

namespace  e
{
	shader_t kGPUImagePinkBlurVertexShaderString = SHADER_STRING
	(
		const int BLUR_SAMPLES = 20;
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;
		
		uniform vec2 offsets[BLUR_SAMPLES];
		varying vec2 blurCoordinates[BLUR_SAMPLES];
		varying vec2 textureCoordinate;

		void main()
		{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;

			for (int i=0; i<BLUR_SAMPLES; i++)
			{
				blurCoordinates[i] = textureCoordinate + offsets[i];
			}
		}
	);

	shader_t kGPUImagePinkBlurFragmentShaderString = SHADER_STRING
	(
		precision highp float;
        const int BLUR_SAMPLES = 20;
		uniform sampler2D inputImageTexture;
		uniform float weights[BLUR_SAMPLES];

		varying highp vec2 textureCoordinate;
		varying highp vec2 blurCoordinates[BLUR_SAMPLES];

		void main()
		{
			float sc = texture2D(inputImageTexture, textureCoordinate).g * 22.0;

			for (int i=0; i<BLUR_SAMPLES; i++)
			{
				sc += texture2D(inputImageTexture, blurCoordinates[i]).g * weights[i];
			}

			sc /= 50.0;
			gl_FragColor = vec4(sc, sc, sc, 1.0);
		}
	);

	GPUImagePinkBlurFilter::GPUImagePinkBlurFilter()
	{
		sampleOffsetsUniform = 0;
		sampleWeightsUniform = 0;
		texelSpacingMultiplier = 0.0f;
		SetTexelSpacingMultiplier(0.5f);
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImagePinkBlurFilter::~GPUImagePinkBlurFilter()
	{

	}

	bool GPUImagePinkBlurFilter::Initialize()
	{
		if (!GPUImageFilter::Initialize(kGPUImagePinkBlurVertexShaderString
			, kGPUImagePinkBlurFragmentShaderString))
		{
			LOGE("GPUImagePinkBlurFilter init failed");
			return false;
		}

		sampleOffsetsUniform = _filterProgram->GetUniformLocation("offsets");
		sampleWeightsUniform = _filterProgram->GetUniformLocation("weights");

		LOGD("GPUImagePinkBlurFilter init ok");
		return true;
	}

	void GPUImagePinkBlurFilter::SetupFilter(Size size)
	{
		if (size.width>0 && size.height>0)
		{
			sampleOffsetWidth = 1.0f / size.width;
			sampleOffsetHeight = 1.0f / size.height;
		}
	}

	void GPUImagePinkBlurFilter::SetProgramUniforms(int index)
	{
		GPUImageFilter::SetProgramUniforms(index);

		// static GLfloat kernels[20][2] = {
		// 	{-10.0f,0.0f},{-8.0f,-5.0f},{-5.0f,-8.0f},{ 0.0f,-10.0f},
		// 	{ 5.0f,-8.0f},{ 8.0f,-5.0f},{10.0f, 0.0f},{ 8.0f,  5.0f},
		// 	{ 5.0f, 8.0f},{ 0.0f,10.0f},{-5.0f, 8.0f},{-8.0f,  5.0f},
		// 	{-6.0f, 0.0f},{-4.0f,-4.0f},{ 0.0f,-6.0f},{ 4.0f, -4.0f},
		// 	{ 6.0f, 0.0f},{ 4.0f, 4.0f},{ 0.0f, 6.0f},{-4.0f,  4.0f}
		// };

		static GLfloat weights[20] = {
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			2.0f, 2.0f, 2.0f, 2.0f,
			2.0f, 2.0f, 2.0f, 2.0f,
		};

		GLfloat offsets[20][2]= {{0,0}};
		for (int i=0; i<20; i++)
		{
			offsets[i][0] = sampleOffsetWidth * texelKernels[i][0];
			offsets[i][1] = sampleOffsetHeight* texelKernels[i][1];
		}

		glUniform2fv(sampleOffsetsUniform, 20, offsets[0]);
		glUniform1fv(sampleWeightsUniform, 20, weights);
	}

	void GPUImagePinkBlurFilter::SetTexelSpacingMultiplier(float multiplier)
	{
		texelSpacingMultiplier = max(0.25f, multiplier);

		const float bases[20][2] = {
			{-10.0f,0.0f},{-8.0f,-5.0f},{-5.0f,-8.0f},{ 0.0f,-10.0f},
			{ 5.0f,-8.0f},{ 8.0f,-5.0f},{10.0f, 0.0f},{ 8.0f,  5.0f},
			{ 5.0f, 8.0f},{ 0.0f,10.0f},{-5.0f, 8.0f},{-8.0f,  5.0f},
			{-6.0f, 0.0f},{-4.0f,-4.0f},{ 0.0f,-6.0f},{ 4.0f, -4.0f},
			{ 6.0f, 0.0f},{ 4.0f, 4.0f},{ 0.0f, 6.0f},{-4.0f,  4.0f}
		};

		int values[2] = {0};
		for (int i=0; i<20; i++)
		{
			values[0] = (int)(bases[i][0] * texelSpacingMultiplier);
			values[1] = (int)(bases[i][1] * texelSpacingMultiplier);
			texelKernels[i][0] = (GLfloat)values[0];
			texelKernels[i][1] = (GLfloat)values[1]; 
		}
	}
}
