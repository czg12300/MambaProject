//
//AnimateManager.h
//
#ifndef E_ANIMEMANAGER_H
#define E_ANIMEMANAGER_H
#include "AnimateGroup.h"
#include <map>

namespace e
{
	class AnimateManager
	{
	public:
		AnimateManager(void);
		virtual ~AnimateManager(void);
	public:
		bool Load(const char* path);
		void SetCacheSize(const int  size);
		void Step(bool e, bool d, GLuint* texture, AnimateConfig** animateConfig);
	protected:
		GLuint Step(bool d, AnimateGroup* animateGroup);
		bool LookupCache(const char* path, AnimateGroup** animateGroup);
		void InsertCache(AnimateGroup* animateGroup);
		void ClearCache(void);
	protected:
		bool _loaded;

		AnimateConfig* emptyConfig;
		AnimateGroup*  emptyGroup;

		AnimateConfig* currentConfig;
		AnimateGroup*  currentGroup;

		//anime cache
		int _cacheSize;
		typedef std::map<uint32_t, AnimateGroup*> AnimateGroupCache;
		AnimateGroupCache _animateCache;
	};
}

#endif