//
// Created by yongfali on 2016/11/14.
//

#include "include/GPUImageHighpassFilter.h"

namespace e
{
	shader_t kGPUImageHighpassVertexShaderString = SHADER_STRING
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
			blurCoordinates[0] = inputTextureCoordinate.xy + vec2(-texelWidthOffset, -texelHeightOffset);
			blurCoordinates[1] = inputTextureCoordinate.xy + vec2(0, -texelHeightOffset);
			blurCoordinates[2] = inputTextureCoordinate.xy + vec2(texelWidthOffset, -texelHeightOffset);

			blurCoordinates[3] = inputTextureCoordinate.xy;
			blurCoordinates[4] = inputTextureCoordinate.xy + vec2(-texelWidthOffset, 0);
			blurCoordinates[5] = inputTextureCoordinate.xy + vec2(texelWidthOffset, 0);

			blurCoordinates[6] = inputTextureCoordinate.xy + vec2(-texelWidthOffset, texelHeightOffset);
			blurCoordinates[7] = inputTextureCoordinate.xy + vec2(texelWidthOffset, texelHeightOffset);
			blurCoordinates[8] = inputTextureCoordinate.xy + vec2(texelWidthOffset, texelHeightOffset);
		}
	);

	shader_t kGPUImageHighpassFragmentShaderString = SHADER_STRING
	(
		precision highp float;
		const int GAUSSIAN_SAMPLES = 9;

		uniform sampler2D inputImageTexture;
		uniform float texelKernels[GAUSSIAN_SAMPLES];

		varying highp vec2 textureCoordinate;
		varying highp vec2 blurCoordinates[GAUSSIAN_SAMPLES];

		void main()
		{
			lowp vec4 cc = vec4(0.0,0.0,0.0,0.0);

			for (int i=0; i<GAUSSIAN_SAMPLES; i++)
			{
				cc += texture2D(inputImageTexture, blurCoordinates[i]) * texelKernels[i];
			}

			gl_FragColor = texture2D(inputImageTexture, textureCoordinate) - cc  + 0.5;
		}
	);

	GPUImageHighpassFilter::GPUImageHighpassFilter()
	{
		_texelKernelsUniform = -1;
		_texelWidthOffsetUniform = -1;
		_texelHeightOffsetUniform = -1;
		_texelWidthOffset = 1.0f / 360;
		_texelHeightOffset = 1.0f / 640;
		_radius = 1;
		_sigma = 1.5f;
		_kernels = 0;
		_isNeedUpdateKernel = false;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageHighpassFilter::~GPUImageHighpassFilter()
	{
		SafeFree(&_kernels);
	}

	bool GPUImageHighpassFilter::Initialize()
	{
		if (!GPUImageFilter::Initialize(kGPUImageHighpassVertexShaderString,
				kGPUImageHighpassFragmentShaderString))
		{
			LOGE("GPUImageHighpassFilter Initialize failed");
			return false;
		}

		_texelKernelsUniform = _filterProgram->GetUniformLocation("texelKernels");
		_texelWidthOffsetUniform = _filterProgram->GetUniformLocation("texelWidthOffset");
		_texelHeightOffsetUniform = _filterProgram->GetUniformLocation("texelHeightOffset");

		CalcKernels(_radius, _sigma);

		LOGD("GPUImageHighpassFilter Initialize ok, %d, %d", _texelWidthOffsetUniform, _texelHeightOffsetUniform);
		return true;
	}

	void GPUImageHighpassFilter::SetupFilter(Size size)
	{
		GPUImageFilter::SetupFilter(size);

		_texelWidthOffset = 1.0f / size.width;
		_texelHeightOffset = 1.0f / size.height;
		//LOGD("GPUImageHighpassFilter setup filter: %d x %d",size.width, size.height);
	}

	void GPUImageHighpassFilter::SetProgramUniforms(int index)
	{
		GPUImageFilter::SetProgramUniforms(index);

		if (_texelWidthOffsetUniform != -1)
		{
			glUniform1f(_texelWidthOffsetUniform, _texelWidthOffset);
		}

		if (_texelHeightOffsetUniform != -1)
		{
			glUniform1f(_texelHeightOffsetUniform, _texelHeightOffset);
		}

		if (_texelKernelsUniform != -1)
		{
			glUniform1fv(_texelKernelsUniform, 9, _kernels);
		}
	}

	void GPUImageHighpassFilter::CalcKernels(int radius, float sigma)
	{
		int size = (2*radius+1) * (2*radius+1);
		_kernels = (GLfloat*)realloc(_kernels, size * sizeof(GLfloat));

		GLfloat sum = 0.0f;
		for (int n = 0, i = -radius; i < radius + 1; i++)
		{
			for(int j= -radius; j < radius + 1; j++, n++)
			{
				float dist = sqrt(i*i + j*j);
				_kernels[n] = (1.0 / sqrt(2.0 * M_PI * pow(sigma, 2.0))) *
							  exp(-pow(dist, 2.0) / (2.0 * pow(sigma, 2.0)));
				sum += _kernels[n];
			}
		}

		for (int i = 0; i < size; i++)
		{
			_kernels[i] /= sum;
			LOGV("kernels[%d] = %f", i, _kernels[i]);
		}
	}
}
