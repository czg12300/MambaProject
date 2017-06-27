#include "include/GPUImageSmoothFilter.h"

namespace e 
{
	inline float HL(float color)
	{
		if (color < 0.5f) {
			color = color * color * 2.0f;
		}else {
			color = 1.0f - ((1.0f - color)*(1.0f - color) * 2.0f);
		}
		return color;
	}

	const char* const kGPUImageSmoothVertexShaderString = SHADER_STRING
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

	const char* const kGPUImageSmoothFragmentShaderString = SHADER_STRING
	(
		precision highp float;
		varying highp vec2 textureCoordinate;

		uniform sampler2D inputImageTexture;
		uniform sampler2D inputImageTexture2;
		uniform sampler2D hardlightTexture;
		uniform vec4 params;
		
		const lowp mat3 saturateMatrix = mat3(
			1.1102, -0.0598, -0.061,
			-0.0774, 1.0826, -0.1186,
			-0.0228, -0.0228, 1.1772);

		void main()
		{
			lowp vec3 cc = texture2D(inputImageTexture, textureCoordinate).rgb;
			lowp vec3 sc = texture2D(inputImageTexture2, textureCoordinate).rgb;

			highp float hp = cc.g - sc.g + 0.5;
			hp = texture2D(hardlightTexture, vec2(hp, 0.0)).r;

			float luminance = dot(cc, vec3(0.299, 0.587, 0.114));
			float alpha = pow(luminance, params.r);

			vec3 sm = cc + (cc - vec3(hp)) * alpha * 0.1;
			sm.r = clamp(pow(sm.r, params.g), 0.0, 1.0);
			sm.g = clamp(pow(sm.g, params.g), 0.0, 1.0);
			sm.b = clamp(pow(sm.b, params.g), 0.0, 1.0);

			vec3 screen = 1.0 - (1.0 - sm) * (1.0 - cc);
			vec3 lighten = max(sm, cc);
#if 1
			vec3 softly = 2.0 * cc * sm + cc*cc - 2.0 * cc * cc * sm;
#else
			vec3 softly = 2.0 * cc * sm + sm*sm - 2.0 * sm * sm * cc;
#endif

			gl_FragColor = vec4(mix(cc, screen, alpha), 1.0);
			gl_FragColor.rgb = mix(gl_FragColor.rgb, lighten, alpha);
			gl_FragColor.rgb = mix(gl_FragColor.rgb, softly, params.b);

			vec3 pc = gl_FragColor.rgb * saturateMatrix;
			gl_FragColor.rgb = mix(gl_FragColor.rgb, pc, params.a);
		}
	);

	GPUImageSmoothFilter::GPUImageSmoothFilter(void)
	{
		_blendParamsUniform = -1;
		_hardlightTextureUniform = -1;
		_hardlightTexture = 0;

		_blendParams[0] = 0.75f;
		_blendParams[1] = 0.90f;
		_blendParams[2] = 0.10f;
		_blendParams[3] = 0.10f;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageSmoothFilter::~GPUImageSmoothFilter(void)
	{

	}

	bool GPUImageSmoothFilter::Initialize(void)
	{
		if (!GPUImageTwoInputFilter::Initialize(kGPUImageSmoothVertexShaderString
			    , kGPUImageSmoothFragmentShaderString))
		{
			LOGE("GPUImageSmoothFilter Initialize failed");
			return false;
		}

		_blendParamsUniform = _filterProgram->GetUniformLocation("params");
		_hardlightTextureUniform = _filterProgram->GetUniformLocation("hardlightTexture");

		LOGV("GPUImageSmoothFilter initialize ok, %d, %d", _blendParamsUniform, _hardlightTextureUniform);
		return true;
	}

	void GPUImageSmoothFilter::UpdateHLTexture(int pass)
	{
		pass = min(pass, 5);

		if (!_hardlightTexture)
		{
			GLubyte samples[1024];
			for (int i = 0; i < 256; i++)
			{
				float temp = i / 255.0f;
				for (int j = 0; j < pass; j++)
				{
					temp = HL(temp);
				}
				GLubyte value = (GLubyte)max(0.0f, min(temp * 255, 255.0f));
				samples[i*4 + 0] = value;
				samples[i*4 + 1] = value;
				samples[i*4 + 2] = value;
				samples[i*4 + 3] = value;
			}

			glGenTextures(1, &_hardlightTexture);
			glBindTexture(GL_TEXTURE_2D, _hardlightTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, samples);

			//glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void GPUImageSmoothFilter::SetProgramUniforms(const int index)
	{
		GPUImageTwoInputFilter::SetProgramUniforms(index);

		if (!_hardlightTexture)
		{
			UpdateHLTexture(4);
		}

		if (_hardlightTextureUniform != -1)
		{
			glUniform4fv(_blendParamsUniform, 1, _blendParams);
		}
	}

	void GPUImageSmoothFilter::SetBlendParams(int index, float value)
	{
		if (index>=0 && index<=3)
		{
			_blendParams[index] = max(0.0f, min(value, 1.0f));
		}
	}

	void GPUImageSmoothFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
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

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, _hardlightTexture);
		glUniform1i(_hardlightTextureUniform, 4);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0,texCoords);
		glVertexAttribPointer(_filterSecondTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
		_secondInputFramebuffer = NULL;
	}

	void GPUImageSmoothFilter::SetSmoothFactor(float value)
	{
		value = 1.0f - max(0.0f, min(value, 1.0f));
		SetBlendParams(0, value);
	}

	void GPUImageSmoothFilter::SetLightenFactor(float value)
	{
		value = max(0.0f, min(value, 1.0f)) * 0.5f;
		value = (1.0f - value) * 0.9f;
		SetBlendParams(1, value);
	}

	void GPUImageSmoothFilter::SetSoftlightFactor(float value)
	{
		value = max(0.0f, min(value, 1.0f)) * 0.5f;
		SetBlendParams(2, value);
	}

	void GPUImageSmoothFilter::SetSaturateFactor(float value)
	{
		value = max(0.0f, min(value, 1.0f)) * 0.3f;
		SetBlendParams(3, value);
	}
}