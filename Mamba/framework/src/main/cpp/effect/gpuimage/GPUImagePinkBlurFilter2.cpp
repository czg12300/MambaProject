//
// Created by yongfali on 2016/11/3.
//

#include "include/GPUImagePinkBlurFilter2.h"
#include <sstream>

namespace e
{
	GPUImagePinkBlurFilter2::GPUImagePinkBlurFilter2()
	{
		_sigma = 4.0f;
		_blurRadius = 4.0f;
		SetTexelSpacingMultiplier(1.0f);
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImagePinkBlurFilter2::~GPUImagePinkBlurFilter2()
	{

	}

	bool GPUImagePinkBlurFilter2::Initialize()
	{
		if (!GPUImageGaussianBlurFilter::Initialize())
		{
			LOGE("GPUImagePinkBlurFilter2 initialize failed!");
			return false;
		}

		LOGV("GPUImagePinkBlurFilter2 initialize ok");
		return true;
	}

	string GPUImagePinkBlurFilter2::CreateFragmentShaderString(int blurRadius, float sigma)
	{
		if (blurRadius < 1)
		{
			return string(kGPUImagePassthroughFragmentShaderString);
		}

		int radius = (int)(blurRadius + 0.5f);
		float* weights = new float[radius + 1];

		float sum = 0.0f;
		for (int i = 0; i < blurRadius + 1; i++)
		{
			weights[i] = (1.0 / sqrt(2.0 * M_PI * pow(sigma, 2.0))) * exp(-pow(i, 2.0) / (2.0 * pow(sigma, 2.0)));
			if (i == 0) {
				sum += weights[i];
			} else {
				sum += 2.0 * weights[i];
			}
		}

		for (int i = 0; i < blurRadius + 1; i++)
		{
			weights[i] = weights[i] / sum;
		}

		int n = min((radius/2) + (radius % 2), 7);
		int m = (radius/2) + (radius%2);

		ostringstream os;
		os << "uniform sampler2D inputImageTexture;" << endl;
		os << "varying highp float texelWidthOffset;" << endl;
		os << "varying highp float texelHeightOffset;" << endl;
		os << "varying highp vec2 blurCoordinates[" << (1 + (n * 2)) << "];" << endl;
		os << "void main()" << endl;
		os << "{" << endl;
		os << "    lowp float sum = texture2D(inputImageTexture, blurCoordinates[0]).g * " << weights[0] << ";" << endl;

		for (int i = 0; i < n; i++)
		{
			float w1 = weights[i * 2 + 1];
			float w2 = weights[i * 2 + 2];
			float w = w1 + w2;
			os << "    sum += texture2D(inputImageTexture, blurCoordinates[" << (i*2)+1 << "]).g * " << w << ";" << endl;
			os << "    sum += texture2D(inputImageTexture, blurCoordinates[" << (i*2)+2 << "]).g * " << w << ";" << endl;
		}

		if (m > n)
		{
			os << "    highp vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);" << endl;

			for (int i = n; i < m; i++)
			{
				float w1 = weights[i * 2 + 1];
				float w2 = weights[i * 2 + 2];
				float w = w1 + w2;
				float offset = (w1 * (i * 2 + 1) + w2 * (i * 2 + 2)) / w;
				os << "    sum += texture2D(inputImageTexture, blurCoordinates[0] + singleStepOffset * " << offset << ").g * " << w << ";" << endl;
				os << "    sum += texture2D(inputImageTexture, blurCoordinates[0] - singleStepOffset * " << offset << ").g * " << w << ";" << endl;
			}
		}

		os << "    gl_FragColor = vec4(sum, sum, sum, 1.0);" << endl;
		os << "}" << endl;

		delete[] weights;
		return os.str();
	}
}
