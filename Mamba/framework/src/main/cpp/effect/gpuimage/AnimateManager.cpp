//
//AnimateManager.h
//
#include "include/AnimateManager.h"
#include "include/AnimateLoader.h"
#include "include/GPUImageBase.h"

namespace e
{
	const int kAnimateCacheSizeDefault = 3;
	AnimateManager::AnimateManager(void)
	{
		_loaded = false;
		emptyGroup = NULL;
		emptyConfig = NULL;
		currentGroup = NULL;
		currentConfig = NULL;

		_cacheSize = kAnimateCacheSizeDefault;
	}

	AnimateManager::~AnimateManager(void)
	{
		SafeDelete(&emptyGroup);
		SafeDelete(&currentGroup);

		ClearCache();
	}

	void AnimateManager::SetCacheSize(const int _size)
	{
		_cacheSize = MAX(0, _size);
	}

	bool AnimateManager::Load(const char* _path)
	{
		CheckPointer(_path, false);

		//生成一个空的动画组
		if (!emptyGroup){
			 emptyGroup = AnimateLoader::Load(NULL);
			if (emptyGroup){
				emptyConfig = &(emptyGroup->config);
			}else{
				emptyConfig = NULL;
				LOGE("AnimateManager load empty anime failed!");
				return false;
			}
		}

		//加载当前动画纹理组
		uint32_t key = GenHashKey(_path, strlen(_path));

		if (_loaded && currentGroup){
			if (currentGroup->hashKey == key){
				return true;
			}
		}

		//从Cache查找
		AnimateGroup* tempGroup = currentGroup;
		if (LookupCache(_path, &currentGroup)){
			if (tempGroup) {
				InsertCache(tempGroup);
			}
			currentConfig = &(currentGroup->config);
			_loaded = true;
			return true;
		}

		if (tempGroup){
			InsertCache(tempGroup);
		}

		//Cache中不存在，从文件读取
		if (!currentGroup){
			currentGroup = AnimateLoader::Load(_path);
			if (currentGroup){
				currentGroup->lasthit = Time::GetTime();
				currentConfig = &(currentGroup->config);
				_loaded = true;
			}else{
				currentConfig = NULL;
				_loaded = false;
			}
		}

		return _loaded;
	}

	void AnimateManager::Step(bool _e, bool _d, GLuint* _animateTexture, AnimateConfig** _animateConfig)
	{
		//生成一个空的动画组
		if (!emptyGroup){
			 emptyGroup = AnimateLoader::Load(NULL);
			if (emptyGroup){
				emptyConfig = &(emptyGroup->config);
				LOGD("AnimateManager create empty animate ok!");
			}else{
				emptyConfig = NULL;
				LOGE("AnimateManager create empty animate failed!");
				return;
			}
		}

		if (_e){
			*_animateTexture = Step(_d, emptyGroup);
			*_animateConfig = emptyConfig;
		}else{
			*_animateTexture = Step(_d, _loaded?currentGroup:emptyGroup);
			*_animateConfig = _loaded?currentConfig:emptyConfig;
		}
	}

	GLuint AnimateManager::Step(bool _d, AnimateGroup* _animateGroup)
	{
		if (!_d){
            _animateGroup->current = (_animateGroup->current + 1) % _animateGroup->textures.size();
		}else{
			uint64_t time = Time::GetTime() - _animateGroup->prevtime;
			if (time > _animateGroup->interval){
                _animateGroup->prevtime += time;
                _animateGroup->current = (_animateGroup->current + 1) % _animateGroup->textures.size();
			}
		}
		//LOGD("current = %d, interval = %d", animeGroup->_current, animeGroup->_interval);
		return _animateGroup->textures[_animateGroup->current].native;
	}

	bool AnimateManager::LookupCache(const char* _path, AnimateGroup** _animateGroup)
	{
		CheckPointer(_path, false);

		uint32_t key = GenHashKey(_path, strlen(_path));
		AnimateGroupCache::iterator it = _animateCache.find(key);
		if(it != _animateCache.end())
		{
			*_animateGroup = it->second;
			_animateCache.erase(it);
			return true;
		}

		*_animateGroup = NULL;
		return false;
	} 

	void AnimateManager::InsertCache(AnimateGroup* _animateGroup)
	{
		//不启用Cache
		if (_cacheSize <= 0)
		{
			SafeDelete(&_animateGroup);
			return;
		}

		//淘汰早期的资源
		if (_animateCache.size() >= (size_t)_cacheSize)
		{
			uint64_t earliest = E_UINT64_MAX;
			AnimateGroupCache::iterator it = _animateCache.begin(), temp = _animateCache.end();
			for (; it != _animateCache.end(); it++)
			{
				AnimateGroup* animate = it->second;
				if (animate->lasthit < earliest){
					earliest = animate->lasthit;
					temp = it;
				}
			}

			if (temp != _animateCache.end())
			{
				SafeDelete(&(temp->second));
				_animateCache.erase(temp);
			}
		}

		_animateCache.insert(std::make_pair(_animateGroup->hashKey, _animateGroup));
	}

	void AnimateManager::ClearCache(void)
	{
		//清理Cache
		AnimateGroupCache::iterator it = _animateCache.begin();
		for (; it != _animateCache.end(); it++)
		{
			SafeDelete(&(it->second));
		}
		_animateCache.clear();
	}
}