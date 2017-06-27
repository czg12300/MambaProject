//
// Created by liyongfa on 2017/3/18.
//

#include "include/ImageWriter.h"

namespace e
{
	ImageWriter::ImageWriter()
	{
		_enableWrite = true;
	}

	ImageWriter::~ImageWriter()
	{

	}

	void ImageWriter::SetEnable(bool enable)
	{
		_enableWrite = enable;
	}

	bool ImageWriter::IsEnable() const 
	{
		return _enableWrite;
	}

	int ImageWriter::OnSampleProc(void* data)
	{
		static int fileCount = 0;

		if (_enableWrite && (++fileCount%25==0))
		{
			int fileIndex = fileCount / 25;
			VideoSample* sample = static_cast<VideoSample*>(data);

			char fileName[256] = {0};
			sprintf(fileName, "%st%03d.bmp", GPUImageEnvironment::GetPath(KG_PATH_OUTPUT).c_str(), fileIndex);

			Bitmap::Save(fileName, sample->width, sample->height, 32, sample->data, sample->size);
			LOGD("save bmp: %d x %d, %s", sample->width, sample->height, fileName);
		}

		return 0;
	}
}
