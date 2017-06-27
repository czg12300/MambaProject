//
// Created by yongfali on 2017/3/16.
//

#include "include/CurveSamples.h"
#include <assert.h>
#include <string.h>

namespace  e
{
	static const int kSampleTypes = 5;
	static const int kCurveChannelCount = 4;
	static const int kSamplePointCount = 9;

	CurveSamples::CurveSamples()
	{
		for (int i=0; i<kSampleTypes; i++)
		{
			double** points = new double*[kCurveChannelCount];
			assert(points);

			for (int j=0; j<kCurveChannelCount; j++)
			{
				points[j] = new double[kSamplePointCount*2];
				assert(points[j]);
			}

			samplePoints[i] = points;
		}

		Initialize();
	}

	CurveSamples::~CurveSamples()
	{
		for (int i=0; i<kSampleTypes; i++)
		{
			double** obj = samplePoints[i];
			assert(obj);

			for (int j = 0; j<kCurveChannelCount; j++)
			{
				double* p = obj[j];
				if (p) delete[] p;
			}

			if (obj){
				delete[] obj;
			}
		}
	}

	bool CurveSamples::GetCurvePoints(int index, int channel, double**points, int *count)
	{
		if (index<0||index>4)
		{
			*count = 0;
			*points = 0;
			return false;
		}
		
		if (channel<0||channel>3)
		{
			*count = 0;
			*points = 0;
			 return false;
		}

		double** pp = samplePoints[index];

		*points = pp[channel];
		*count = kSamplePointCount;
		return true;
	}

	void CurveSamples::Initialize()
	{
		//0:
		{
			double pts[] = {
				0.000000, 0.007843,
				0.121569, 0.160784,
				0.247059, 0.317647,
				0.372549, 0.462745,
				0.498039, 0.592157,
				0.623529, 0.713725,
				0.749020, 0.819608,
				0.874510, 0.913725,
				1.000000, 0.996078,
			};

			double** pp = samplePoints[0];
			memcpy(pp[0], pts, sizeof(pts));
			memcpy(pp[1], pts, sizeof(pts));
			memcpy(pp[2], pts, sizeof(pts));
		}
	
		//1:
		{
			double r_pts[] = {
				0.000000, 0.007843,
				0.121569, 0.192157,
				0.247059, 0.372549,
				0.372549, 0.529412,
				0.498039, 0.666667,
				0.623529, 0.784314,
				0.749020, 0.874510,
				0.874510, 0.945098,
				1.000000, 0.996078,
			};

			double g_pts[] ={
				0.000000, 0.007843,
				0.121569, 0.160784,
				0.247059, 0.321569,
				0.372549, 0.470588,
				0.498039, 0.600000,
				0.623529, 0.721569,
				0.749020, 0.827451,
				0.874510, 0.917647,
				1.000000, 0.996078,
			};

			double b_pts[] = {
				0.000000, 0.007843,
				0.121569, 0.160784,
				0.247059, 0.321569,
				0.372549, 0.470588,
				0.498039, 0.600000,
				0.623529, 0.721569,
				0.749020, 0.827451,
				0.874510, 0.917647,
				1.000000, 0.996078,
			};

			double** pp = samplePoints[1];
			memcpy(pp[0], r_pts, sizeof(r_pts));
			memcpy(pp[1], g_pts, sizeof(g_pts));
			memcpy(pp[2], b_pts, sizeof(b_pts));
		}
		//2:
		{
			double r_pts[] = {
				0.000000, 0.093137,
				0.121569, 0.125134,
				0.247059, 0.227000,
				0.372549, 0.372794,
				0.498039, 0.537491,
				0.623529, 0.706434,
				0.749020, 0.852155,
				0.874510, 0.953969,
				1.000000, 0.996078,
			};

			double g_pts[] ={
				0.000000, 0.092647,
				0.121569, 0.125205,
				0.247059, 0.227129,
				0.372549, 0.372871,
				0.498039, 0.537711,
				0.623529, 0.706357,
				0.749020, 0.851153,
				0.874510, 0.953240,
				1.000000, 0.996078,
			};

			double b_pts[] = {
				0.000000, 0.003922,
				0.121569, 0.029810,
				0.247059, 0.143778,
				0.372549, 0.305764,
				0.498039, 0.488796,
				0.623529, 0.672134,
				0.749020, 0.833704,
				0.874510, 0.948010,
				1.000000, 0.996078,
			};

			double** pp = samplePoints[2];
			memcpy(pp[0], r_pts, sizeof(r_pts));
			memcpy(pp[1], g_pts, sizeof(g_pts));
			memcpy(pp[2], b_pts, sizeof(b_pts));
		}

		//3:
		{
			double r_pts[] = {
				0.000000, 0.007843,
				0.121569, 0.141176,
				0.247059, 0.286275,
				0.372549, 0.423529,
				0.498039, 0.552941,
				0.623529, 0.674510,
				0.749020, 0.792157,
				0.874510, 0.898039,
				1.000000, 1.000000,
			};

			double g_pts[] ={
				0.000000, 0.007843,
				0.121569, 0.184314,
				0.247059, 0.360784,
				0.372549, 0.517647,
				0.498039, 0.654902,
				0.623529, 0.768627,
				0.749020, 0.866667,
				0.874510, 0.945098,
				1.000000, 1.000000,
			};

			double b_pts[] = {
				0.000000, 0.007843,
				0.121569, 0.211765,
				0.247059, 0.407843,
				0.372549, 0.576471,
				0.498039, 0.717647,
				0.623529, 0.827451,
				0.749020, 0.913725,
				0.874510, 0.972549,
				1.000000, 1.000000,
			};

			double** pp = samplePoints[3];
			memcpy(pp[0], r_pts, sizeof(r_pts));
			memcpy(pp[1], g_pts, sizeof(g_pts));
			memcpy(pp[2], b_pts, sizeof(b_pts));
		}
		//4:
		{
			double r_pts[] = {
				0.000000, 0.003922,
				0.121569, 0.181978,
				0.247059, 0.348183,
				0.372549, 0.498116,
				0.498039, 0.630567,
				0.623529, 0.746150,
				0.749020, 0.848447,
				0.874510, 0.939235,
				1.000000, 1.000000,
			};

			double g_pts[] ={
				0.000000, 0.002451,
				0.121569, 0.182281,
				0.247059, 0.347678,
				0.372549, 0.495968,
				0.498039, 0.629733,
				0.623529, 0.747508,
				0.749020, 0.852278,
				0.874510, 0.937573,
				1.000000, 1.000000,
			};

			double b_pts[] = {
				0.000000, 0.005240,
				0.121569, 0.182529,
				0.247059, 0.347919,
				0.372549, 0.496757,
				0.498039, 0.638620,
				0.623529, 0.753394,
				0.749020, 0.852676,
				0.874510, 0.948770,
				1.000000, 1.000000,
			};

			double** pp = samplePoints[4];
			memcpy(pp[0], r_pts, sizeof(r_pts));
			memcpy(pp[1], g_pts, sizeof(g_pts));
			memcpy(pp[2], b_pts, sizeof(b_pts));
		}
	}
}
