//
// Created by yongfali on 2016/4/21.
//
#ifndef E_GPUIMAGE_MATH_H
#define E_GPUIMAGE_MATH_H

#include <math.h>
#include "Matrixs.h"

namespace e
{
	//计算正交投影矩阵
	void CalcOrtho2D(Matrix4 & mat
		, float left
		, float right
		, float bottom
		, float top
		, float nearVal
		, float farVal);

	//计算透视投影矩阵
	void CalcPerspective(Matrix4 & mat
		, float fovyArc
		, float aspect
		, float nearVal
		, float farVal);

	//计算观察矩阵
	void CalcLookAt(Matrix4 & mat
		, const Vector3 & eye
		, const Vector3 & lookAt
		, const Vector3 & up);
}

#endif
