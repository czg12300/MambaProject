//
// Created by yongfali on 2016/3/25.
//

#ifndef E_GPUIMAGE_BASE_H
#define E_GPUIMAGE_BASE_H

#include "GPUImageEnvironment.h"
#include "GPUImageCommon.h"
#include "GPUImageRotationMode.h"
#include "GPUImageTexture.h"
#include "GPUImageFramebuffer.h"
#include "GPUImageContext.h"
#include "GLProgram.h"

namespace e 
{
	typedef const char* const shader_t;

	void GetOpenGLError(const char* operation, const char* function);

#ifndef CheckOpenGLError
#	define CheckOpenGLError(op) GetOpenGLError((op), __FUNCTION__)
#endif

}

#endif //E_GPUIMAGE_BASE_H
