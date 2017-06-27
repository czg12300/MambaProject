//
// Created by yongfali on 2016/4/13.
//

#ifndef E_ANIMEGROUP_H
#define E_ANIMEGROUP_H

#include <string.h>
#include <string>
#include <vector>
#include <GLES2/gl2.h>

namespace e
{
	struct AnimateConfig
	{
		int type;
		int layers;
		int duration;
		int ref_point;
		float width;
		float height;
		float offset_x;
		float offset_y;
		float ref_eye_dist;
		float ref_face_size;
	};

	struct AnimateTexture
	{
		GLuint width;
		GLuint height;
		GLuint native;
	};

	struct AnimateGroup
	{
		uint32_t hashKey;
		uint32_t current;
		uint64_t interval;
		uint64_t prevtime;
		uint64_t lasthit;

		AnimateConfig config;
		std::vector<AnimateTexture> textures;
	};
}

#endif