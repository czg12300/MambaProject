#ifndef E_GPUIMAGE_TEXTUREOPTIONS_H
#define E_GPUIMAGE_TEXTUREOPTIONS_H
#include "GPUImageCommon.h"

namespace e
{
	class GPUImageTextureOptions 
	{
	public:
		GPUImageTextureOptions(void);
		GPUImageTextureOptions(GLenum minFilter
			, GLenum magFilter
			, GLenum wrapS
			, GLenum wrapT
			, GLenum internalFormat
			, GLenum format
			, GLenum type);
		GPUImageTextureOptions(const GPUImageTextureOptions & options);
		virtual ~GPUImageTextureOptions(void);
	public:
		GLenum minFilter;
		GLenum magFilter;
		GLenum wrapS;
		GLenum wrapT;
		GLenum internalFormat;
		GLenum format;
		GLenum type;
	};

	void SetTextureOptionsDefault(GPUImageTextureOptions & textureOptions);
	void SetTextureOptionsLuminance(GPUImageTextureOptions & textureOptions);
	void SetTextureOptionsChrominance(GPUImageTextureOptions & textureOptions);	
}
#endif //E_GPUIMAGE_TEXTUREOPTIONS_H