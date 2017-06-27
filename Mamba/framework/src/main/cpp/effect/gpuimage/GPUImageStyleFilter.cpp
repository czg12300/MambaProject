//
// Created by yongfali on 2017/3/16.
//

#include "include/GPUImageStyleFilter.h"

namespace e
{
	static const int kCurveChannels = 4;
	static const int kSamplePointCount = 9;

	shader_t kGPUImageStyleVertexShaderString = SHADER_STRING
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

	shader_t kGPUImageStyleFragmentShaderString = SHADER_STRING
	(
		varying highp vec2 textureCoordinate;

		uniform sampler2D inputImageTexture; //rgba sampler texture
		uniform sampler2D curveImageTexture; //curve sampler texture

		void main()
		{
			lowp vec4 color = texture2D(inputImageTexture, textureCoordinate);
			lowp float r = texture2D(curveImageTexture, vec2(color.r, 0.0)).r;
			lowp float g = texture2D(curveImageTexture, vec2(color.g, 0.0)).g;
			lowp float b = texture2D(curveImageTexture, vec2(color.b, 0.0)).b;
			gl_FragColor = vec4(r, g, b, color.a);
		}
	);

	GPUImageStyleFilter::GPUImageStyleFilter()
	{
		_curveTexture = 0;
		_filterCurveTextureUniform = -1;
#ifdef TEXTURE_CACHE_ENABLE			
		_textureCache = GPUImageTextureCache::Singleton();
#endif		
        _isNeedUpdate = true;

		for (int i=0; i<kCurveChannels; i++)
		{
			_samplePoints[i] = new double[kSamplePointCount*2];
			memset(_samplePoints[i], 0, sizeof(double)*kSamplePointCount*2);
		}

		MakeSamplesDefault();

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageStyleFilter::~GPUImageStyleFilter()
	{
		for (int i=0; i<kCurveChannels; i++)
		{
			if (_samplePoints[i])
			{
				delete[] _samplePoints[i];
				_samplePoints[i] = 0;
			}
		}

#ifndef TEXTURE_CACHE_ENABLE
		if (eglGetCurrentContext() != _eglContext)
		{
			return;
		}

		if (_curveTexture)
		{
			glDeleteTextures(1, &_curveTexture);
			_curveTexture = 0;
		}
#else
		_curveTexture = 0;
		_textureCache = 0;
#endif
	}

	bool GPUImageStyleFilter::Initialize()
	{
		if (!GPUImageFilter::Initialize(kGPUImageStyleVertexShaderString, kGPUImageStyleFragmentShaderString))
		{
			LOGE("GPUImageStyleFilter Initialize failed");
			return false;
		}

		_filterCurveTextureUniform = _filterProgram->GetUniformLocation("curveImageTexture");
		LOGV("GPUImageStyleFilter initialize ok : %d", _filterCurveTextureUniform);
		return true;
	}

	void GPUImageStyleFilter::SetSamplePoints(int channel, double *points, int count)
	{
		assert(count == kSamplePointCount);
		if (channel<0||channel>3)return;

		if (points != 0) {
			count = min(kSamplePointCount, count);
			memcpy(_samplePoints[channel], points, sizeof(double) * count * 2);
		}else{
			MakeSamplesDefault();
		}

		_isNeedUpdate = true;
	}

	void GPUImageStyleFilter::MakeSamplesDefault()
	{
		double points[] = {
			0.000000, 0.000000,
			-1.0, -1.0,
			-1.0, -1.0,
			-1.0, -1.0,
			-1.0, -1.0,
			-1.0, -1.0,
			-1.0, -1.0,
			-1.0, -1.0,
			1.000000, 1.000000
		};

		for (int i=0; i<kCurveChannels; i++)
		{
			memcpy(_samplePoints[i], points, sizeof(points));
		}
	}

	void GPUImageStyleFilter::UpdateCurveTexture()
	{
		byte samples[256] = { 0 };
		byte pixels[1024] = { 0 };

		//only rgb channels
		for (int j=0; j<kCurveChannels; j++)
		{
			if (j < 3)//RGB
			{
				Calculate(samples, 256, _samplePoints[j], 9);

				for (int i = 0; i < 256; i++)
				{
					pixels[i * 4 + j] = samples[i];
				}
			}
			else//alpha
			{
				for (int i = 0; i < 256; i++)
				{
					pixels[i * 4 + j] = i;
				}
			}
		}
#ifndef TEXTURE_CACHE_ENABLE
		if (_curveTexture == 0)
		{
			glGenTextures(1, &_curveTexture);
			_eglContext = eglGetCurrentContext();
		}
		glBindTexture(GL_TEXTURE_2D, _curveTexture);	
#else
		if (_curveTexture == 0)
        {
			Size texSize = Size(256, 1);
            _curveTexture = _textureCache->Fetch(texSize
                , kGPUImageTextureOptionsDefault
                , 3);
        }
		glBindTexture(GL_TEXTURE_2D, _curveTexture->GetTexture());
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		//glBindTexture(GL_TEXTURE_2D, 0);
		_isNeedUpdate = false;
	}

	void GPUImageStyleFilter::RenderToTexture(const float *vertices, const float *texCoords)
	{
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		if (_isNeedUpdate)
		{
			UpdateCurveTexture();
		}

		SetProgramUniforms(0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glActiveTexture(GL_TEXTURE3);
#ifndef TEXTURE_CACHE_ENABLE		
		glBindTexture(GL_TEXTURE_2D, _curveTexture);
#else
		glBindTexture(GL_TEXTURE_2D, _curveTexture->GetTexture());
#endif
		glUniform1i(_filterCurveTextureUniform, 3);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
	}
	//calculate by bezier curve range[0, 255]
	int GPUImageStyleFilter::Calculate(byte* samples
		, const int sampleCount
		, const double* points
		, const int pointCount)
	{
		double* _samples = new double[sampleCount];
		int result = Calculate(_samples, sampleCount, points, pointCount);
		for (int i = 0; i < sampleCount; i++){
			samples[i] = (byte)ROUND(_samples[i] * 255);
		}
		delete[] _samples;
		return result;
	}

	int GPUImageStyleFilter::Calculate(double* samples
		, const int sampleCount
		, const double* points
		, const int pointCount)
	{
		int p1, p2, p3, p4;
		int count = 0;
		double x, y;

		int* p = (int*)malloc(pointCount * sizeof(int));
		assert(p);
		if (!p) return -1;

		for (int i = 0; i < pointCount; i++)
		{
			x = points[2 * i + 0];
			y = points[2 * i + 1];

			if (x >= 0.0)
			{
				p[count++] = i;
			}
		}

		if (count != 0)
		{
			x = points[2 * p[0] + 0];
			y = points[2 * p[0] + 1];
			int boundary = ROUND(x * (double)(sampleCount - 1));
			for (int i = 0; i < boundary; i++)
			{
				samples[i] = y;
			}

			x = points[2 * p[count - 1] + 0];
			y = points[2 * p[count - 1] + 1];
			boundary = ROUND(x * (double)(sampleCount - 1));
			for (int i = boundary; i < sampleCount; i++)
			{
				samples[i] = y;
			}

			for (int i = 0; i < count - 1; i++)
			{
				p1 = p[MAX(i - 1, 0)];
				p2 = p[i + 0];
				p3 = p[i + 1];
				p4 = p[MIN(i + 2, count - 1)];

				Plot(samples, sampleCount, points, pointCount, p1, p2, p3, p4);
			}

			for (int i = 0; i < count; i++)
			{
				x = points[2 * p[i] + 0];
				y = points[2 * p[i] + 1];
				samples[ROUND(x * (double)(sampleCount - 1))] = y;
			}
		}

		if (p) free(p);
		return sampleCount;
	}

	void GPUImageStyleFilter::Plot(double* samples
		, const int sampleCount
		, const double* points
		, const int pointCount
		, int p1
		, int p2
		, int p3
		, int p4)
	{
		double y1, y2, slope;
		double x0 = points[p2 * 2 + 0];
		double y0 = points[p2 * 2 + 1];
		double x3 = points[p3 * 2 + 0];
		double y3 = points[p3 * 2 + 1];

		/*
		* the x values of the inner control points are fixed at
		* x1 = 2/3*x0 + 1/3*x3   and  x2 = 1/3*x0 + 2/3*x3
		* this ensures that the x values increase linearily with the
		* parameter t and enables us to skip the calculation of the x
		* values altogehter - just calculate y(t) evenly spaced.
		*/

		double dx = x3 - x0;
		double dy = y3 - y0;

		if (dx <= 0) return;

		if (p1 == p2 && p3 == p4)
		{
			/* No information about the neighbors,
			* calculate y1 and y2 to get a straight line
			*/
			y1 = y0 + dy / 3.0;
			y2 = y0 + dy * 2.0 / 3.0;
		}
		else if (p1 == p2 && p3 != p4)
		{
			/* only the right neighbor is available. Make the tangent at the
			* right endpoint parallel to the line between the left endpoint
			* and the right neighbor. Then point the tangent at the left towards
			* the control handle of the right tangent, to ensure that the curve
			* does not have an inflection point.
			*/
			slope = (points[p4 * 2 + 1] - y0) / (points[p4 * 2 + 0] - x0);

			y2 = y3 - slope * dx / 3.0;
			y1 = y0 + (y2 - y0) / 2.0;
		}
		else if (p1 != p2 && p3 == p4)
		{
			/* see previous case */
			slope = (y3 - points[p1 * 2 + 1]) / (x3 - points[p1 * 2 + 0]);

			y1 = y0 + slope * dx / 3.0;
			y2 = y3 + (y1 - y3) / 2.0;
		}
		else /* (p1 != p2 && p3 != p4) */
		{
			/* Both neighbors are available. Make the tangents at the endpoints
			* parallel to the line between the opposite endpoint and the adjacent
			* neighbor.
			*/
			slope = (y3 - points[p1 * 2 + 1]) / (x3 - points[p1 * 2 + 0]);

			y1 = y0 + slope * dx / 3.0;

			slope = (points[p4 * 2 + 1] - y0) / (points[p4 * 2 + 0] - x0);

			y2 = y3 - slope * dx / 3.0;
		}

		/*
		* finally calculate the y(t) values for the given bezier values. We can
		* use homogenously distributed values for t, since x(t) increases linearily.
		*/
		for (int i = 0; i <= ROUND(dx * (double)(sampleCount - 1)); i++)
		{
			double t = i / dx / (double)(sampleCount - 1);
			double y = y0 * (1 - t) * (1 - t) * (1 - t) +
					   3 * y1 * (1 - t) * (1 - t) * t +
					   3 * y2 * (1 - t) * t * t +
					   y3 * t * t * t;

			int index = i + ROUND(x0 * (double)(sampleCount - 1));

			if (index < sampleCount)
			{
				samples[index] = clamp(y, 0.0, 1.0);
			}
		}
	}
}
