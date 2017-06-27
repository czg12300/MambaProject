//
// Created by yongfali on 2016/4/13.
//

#ifndef E_FACEEFFECT_H
#define E_FACEEFFECT_H
#include "GPUImageBase.h"
#include "AnimateGroup.h"

namespace e
{
	class AnimateLoader
	{
	public:
		static AnimateGroup* Load(const char* path);
	protected:
		static bool Load(const char* path, AnimateConfig* animateConfig);
		static bool Load(const char* path, AnimateGroup* animateGroup);
		static bool CreateEmptyConfig(AnimateConfig* animateConfig);
		static bool CreateEmptyImage(AnimateGroup* animateGroup);
	};
}

#endif //HIVIDEO_FACE_ANDROID_FACEEFFECT_H
