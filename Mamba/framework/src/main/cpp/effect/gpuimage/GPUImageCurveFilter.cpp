#include "include/GPUImageCurveFilter.h"
#include "include/GPUImageCommon.h"

namespace e 
{
	shader_t kGPUImageCurveVertexShaderString = SHADER_STRING
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

	shader_t kGPUImageCurveFragmentShaderString = SHADER_STRING
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

	GPUImageCurveFilter::GPUImageCurveFilter(void)
	{
		_curveTexture = 0;
		_filterCurveTextureUniform = -1;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageCurveFilter::~GPUImageCurveFilter(void)
	{
		if (_curveTexture)
		{
			glDeleteTextures(1, &_curveTexture);
			_curveTexture = 0;
		}
	}

	bool GPUImageCurveFilter::MakeCurveTexture(void)
	{
		double points[18] = {
			0.000000, 0.000000,
			0.125000, 0.125000,
			0.250000, 0.250000,
			0.375000, 0.375000,
			0.500000, 0.500000,
			0.625000, 0.625000,
			0.750000, 0.750000,
			0.875000, 0.875000,
			1.000000, 1.000000
		};
		byte samples[256] = { 0 };
		byte pixels[1024] = { 0 };
		Calculate(samples, 256, points, 9);

		for (int i = 0; i < 256; i++)
		{
			pixels[i * 4 + 0] = samples[i];
			pixels[i * 4 + 1] = samples[i];
			pixels[i * 4 + 2] = samples[i];
			pixels[i * 4 + 3] = samples[i];
		}

		glGenTextures(1, &_curveTexture);
		glBindTexture(GL_TEXTURE_2D, _curveTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	bool GPUImageCurveFilter::Initialize(void)
	{
		if (!GPUImageFilter::Initialize(kGPUImageCurveVertexShaderString, kGPUImageCurveFragmentShaderString))
		{
			LOGE("GPUImageCurveFilter Initialize failed");
			return false;
		}

		_filterCurveTextureUniform = _filterProgram->GetUniformLocation("curveImageTexture");

		MakeCurveTexture();

		LOGD("GPUImageCurveFilter initialize ok : %d", _filterCurveTextureUniform);
		return true;
	}

	void GPUImageCurveFilter::RenderToTexture(const float* vertices, const float* texCoords)
	{
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		SetProgramUniforms(0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _curveTexture);
		glUniform1i(_filterCurveTextureUniform, 3);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
	}

	//calculate by bezier curve range[0, 255]
	int GPUImageCurveFilter::Calculate(byte* samples
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

	int GPUImageCurveFilter::Calculate(double* samples
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

	void GPUImageCurveFilter::Plot(double* samples
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