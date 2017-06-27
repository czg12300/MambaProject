//
// Created by yongfali on 2017/3/16.
//

#ifndef KUGOUEFFECT_CURVESAMPLES_H
#define KUGOUEFFECT_CURVESAMPLES_H
#include <vector>

namespace e
{
	class CurveSamples
	{
	public:
		CurveSamples(void);
		~CurveSamples(void);
	public:
		bool GetCurvePoints(int index, int channel, double** points, int* count);
	private:
		void Initialize(void);
		double** samplePoints[5];
	};
}

#endif //KUGOUEFFECT_CURVESAMPLES_H
