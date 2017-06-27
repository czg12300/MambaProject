#include "include/GPUImageTextureOptions.h"

namespace e
{
	GPUImageTextureOptions::GPUImageTextureOptions(void)
	{
		SetTextureOptionsDefault(*this);
	}

	GPUImageTextureOptions::GPUImageTextureOptions(GLenum minFilter
       , GLenum magFilter
       , GLenum wrapS
       , GLenum wrapT
       , GLenum internalFormat
       , GLenum format
       , GLenum type)
	{
		this->minFilter = minFilter;
		this->magFilter = magFilter;
		this->wrapS = wrapS;
		this->wrapT = wrapT;
		this->internalFormat = internalFormat;
		this->format = format;
		this->type = type;
	}

	GPUImageTextureOptions::GPUImageTextureOptions(const GPUImageTextureOptions & r)
	{
		this->minFilter = r.minFilter;
		this->magFilter = r.magFilter;
		this->wrapS = r.wrapS;
		this->wrapT = r.wrapT;
		this->internalFormat = r.internalFormat;
		this->format = r.format;
		this->type = r.type;
	}

	GPUImageTextureOptions::~GPUImageTextureOptions(void)
	{

	}

	void SetTextureOptionsDefault(GPUImageTextureOptions & textureOptions)
	{
		textureOptions.minFilter = GL_LINEAR;
		textureOptions.magFilter = GL_LINEAR;
		textureOptions.wrapS = GL_CLAMP_TO_EDGE;
		textureOptions.wrapT = GL_CLAMP_TO_EDGE;
		textureOptions.internalFormat = GL_RGBA;
		textureOptions.format = GL_RGBA;
		textureOptions.type = GL_UNSIGNED_BYTE;
	}

	void SetTextureOptionsLuminance(GPUImageTextureOptions & textureOptions)
	{
		textureOptions.minFilter = GL_LINEAR;
		textureOptions.magFilter = GL_LINEAR;
		textureOptions.wrapS = GL_CLAMP_TO_EDGE;
		textureOptions.wrapT = GL_CLAMP_TO_EDGE;
		textureOptions.internalFormat = GL_LUMINANCE;
		textureOptions.format = GL_LUMINANCE;
		textureOptions.type = GL_UNSIGNED_BYTE;
	}

	void SetTextureOptionsChrominance(GPUImageTextureOptions & textureOptions)
	{
		textureOptions.minFilter = GL_LINEAR;
		textureOptions.magFilter = GL_LINEAR;
		textureOptions.wrapS = GL_CLAMP_TO_EDGE;
		textureOptions.wrapT = GL_CLAMP_TO_EDGE;
		textureOptions.internalFormat = GL_LUMINANCE_ALPHA;
		textureOptions.format = GL_LUMINANCE_ALPHA;
		textureOptions.type = GL_UNSIGNED_BYTE;
	}	
}