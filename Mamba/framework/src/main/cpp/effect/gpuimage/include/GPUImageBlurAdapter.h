#ifndef GPUIMAGE_BLURFADAPTER_H
#define GPUIMAGE_BLURFADAPTER_H

namespace e
{
	class GPUImageBlurAdapter
		: public GPUImageInput
		, public GPUImageOutput
	{
	public:
		GPUImageBlurAdapter();
		virtual ~GPUImageBlurAdapter();
	public:

	};
}

#endif