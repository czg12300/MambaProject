//
// Created by yongfali on 2016/4/13.
//

#include "include/AnimateLoader.h"
#include "png.h"

#define E_UINT64_MAX 0xffffffffffffffff
#define PNG_BYTES_TO_CHECK 4

namespace e
{
	static bool LoadPNG(const char *fileName, char **data, int &width, int &height, int &bitCount)
	{
		FILE *fp;
		png_structp png_ptr;
		png_infop info_ptr;
		png_bytep *row_pointers;
		char buf[PNG_BYTES_TO_CHECK];
		int w, h, x, y, temp, colorType;

		fp = fopen(fileName, "rb");
		if (fp == NULL)
		{
			LOGE("open png file failed! %s", fileName);
			return false;
		}

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);

		setjmp(png_jmpbuf(png_ptr));
		/* 读取PNG_BYTES_TO_CHECK个字节的数据 */
		temp = fread(buf, 1, PNG_BYTES_TO_CHECK, fp);
		/* 若读到的数据并没有PNG_BYTES_TO_CHECK个字节 */
		if (temp < PNG_BYTES_TO_CHECK)
		{
			fclose(fp);
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return false;
		}
		/* 检测数据是否为PNG的签名 */
		temp = png_sig_cmp((png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK);
		/* 如果不是PNG的签名，则说明该文件不是PNG文件 */
		if (temp != 0)
		{
			fclose(fp);
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return false;
		}

		/* 复位文件指针 */
		rewind(fp);
		/* 开始读文件 */
		png_init_io(png_ptr, fp);
		/* 读取PNG图片信息 */
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
		/* 获取图像的色彩类型 */
		colorType = png_get_color_type(png_ptr, info_ptr);
		/* 获取图像的宽高 */
		w = png_get_image_width(png_ptr, info_ptr);
		h = png_get_image_height(png_ptr, info_ptr);
		/* 获取图像的所有行像素数据，row_pointers里边就是rgba数据 */
		row_pointers = png_get_rows(png_ptr, info_ptr);
		/* 根据不同的色彩类型进行相应处理 */
		char *ptr = (char *)malloc(w * h * 4);
		assert(ptr);

		switch (colorType)
		{
		case PNG_COLOR_TYPE_RGB_ALPHA:
			for (y = 0; y < h; ++y)
			{
				char *dst = ptr + y * WidthBytes(w * 32);
				for (x = 0; x < w * 4;)
				{
					*dst++ = row_pointers[y][x++]; // red
					*dst++ = row_pointers[y][x++]; // green
					*dst++ = row_pointers[y][x++]; // blue
					*dst++ = row_pointers[y][x++]; // alpha
				}
			}
			bitCount = 32;
		break;
		case PNG_COLOR_TYPE_RGB:
			for (y = 0; y < h; ++y)
			{
				char *dst = ptr + y * WidthBytes(w * 24);
				for (x = 0; x < w * 3;)
				{
					*dst++ = row_pointers[y][x++]; // red
					*dst++ = row_pointers[y][x++]; // green
					*dst++ = row_pointers[y][x++]; // blue
				}
			}
			bitCount = 24;
			break;
		/* 其它色彩类型的图像就不读了 */
		default:
			*data = 0;
			width = 0;
			height = 0;
			bitCount = 0;
			free(ptr);
			fclose(fp);
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return false;
		}
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);

		*data = ptr;
		width = w;
		height = h;
		return true;
	}

	static std::string GetExtension(const std::string &file)
	{
		return file.substr(file.rfind('.') + 1);
	}

	static bool EnumFiles(std::vector<std::string> &result, const char *path, const char *ext = NULL)
	{
		CheckPointer(path, false);

		DIR *dir = opendir(path);
		CheckPointer(dir, false);

		struct dirent *d = NULL;
		while ((d = readdir(dir)) != NULL)
		{
			if (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			{
				continue;
			}
			// extension filter
			if (ext != NULL)
			{
				std::string name(d->d_name);
				std::size_t pos = name.rfind('.');
				if (pos != std::string::npos)
				{
					std::string extension = name.substr(pos + 1);
					if (extension != ext) continue;
				}
			}

			char fileName[1024] = {0};
			char ch = path[strlen(path) - 1];
			if (ch == '/' || ch == '\\')
			{
				sprintf(fileName, "%s%s", path, d->d_name);
			}
			else
			{
				sprintf(fileName, "%s/%s", path, d->d_name);
			}
			result.push_back(std::string(fileName));
		}

		if (dir)
		closedir(dir);
		return true;
	}

	// effect directroy
	AnimateGroup* AnimateLoader::Load(const char *_path)
	{
		if (!_path)
		{
			AnimateGroup *animeGroup = new AnimateGroup();
			CheckPointer(animeGroup, NULL);
			CreateEmptyConfig(&(animeGroup->config));
			CreateEmptyImage(animeGroup);
			return animeGroup;
		}
		else
		{
			struct stat buf;
			if (lstat(_path, &buf) < 0)
			{
				LOGE("lstat path error: %s", _path);
				return NULL;
			}
			//只对目录有效
			if (!S_ISDIR(buf.st_mode))
			{
				LOGE("AnimateGroup load's path not a directory!!!");
				return NULL;
			}

			AnimateGroup *animateGroup = new AnimateGroup();
			CheckPointer(animateGroup, NULL);

			std::vector<std::string> files;
			//获取该目录下的文件
			EnumFiles(files, _path);
			//对文件名字进行排序
			std::sort(files.begin(), files.end());

			std::vector<std::string>::iterator it = files.begin();
			for (; it != files.end(); it++)
			{
				std::string file = *it;
				std::string ext = GetExtension(file);
				if (ext == "xml")
				{
					Load(file.c_str(), &(animateGroup->config));
				}
				else if (ext == "png")
				{
					Load(file.c_str(), animateGroup);
				}
				else
				{
					LOGW("unknown file : %s", file.c_str());
				}
				// LOGD("load file: %s", file.c_str());
			}

			animateGroup->current = 0;
			animateGroup->prevtime = 0;
			animateGroup->hashKey = GenHashKey(_path, strlen(_path));
			animateGroup->interval = animateGroup->config.duration / MAX(animateGroup->textures.size(), 1);
			animateGroup->lasthit = Time::GetTime();

			LOGD("AnimateGroup load ok -> %s", _path);
			return animateGroup;
		}
	}
	//读取配置文件
	bool AnimateLoader::Load(const char *path, AnimateConfig *_config)
	{
		TiXmlDocument doc(path);
		if (!doc.LoadFile()) return false;

		TiXmlElement *root = doc.RootElement();
		if (root != NULL)
		{
			TiXmlElement *next = root->FirstChildElement();

			while (next != NULL)
			{
				const char *value = next->Value();
				const char *text = next->GetText();

				if (!strcmp(value, "type")){
					_config->type = atoi(text);
				}else if (!strcmp(value, "layers")){
					_config->layers = atoi(text);
				}else if (!strcmp(value, "duration")){
					_config->duration = atoi(text);
				}else if (!strcmp(value, "width")){
					_config->width = atof(text);
				}else if (!strcmp(value, "height")){
					_config->height = atof(text);
				}else if (!strcmp(value, "offset_x")){
					_config->offset_x = atof(text);
				}else if (!strcmp(value, "offset_y")){
					_config->offset_y = atof(text);
				}else if (!strcmp(value, "ref_point")){
					_config->ref_point = atoi(text);
				}else if (!strcmp(value, "ref_eye_dist")){
					_config->ref_eye_dist = atof(text);
				}else if (!strcmp(value, "ref_face_size")){
					_config->ref_face_size = atof(text);
				}
				next = next->NextSiblingElement();
			}
			// normalize offset
			_config->offset_x /= _config->width / 2;
			_config->offset_y /= _config->height / 2;
			return true;
		}

		return false;
	}
	//读取特效图片
	bool AnimateLoader::Load(const char* _path, AnimateGroup* _animateGroup)
	{
		char *data = NULL;
		int width = 0, height = 0, bitCount = 0;
		//读png图片
		if (!LoadPNG(_path, &data, width, height, bitCount))
		{
			LOGE("load anime image failed! %s", _path);
			return false;
		}

		//这些OpenGL代码应该在同OpenGL渲染线程中执行
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		AnimateTexture object;
		object.width = width;
		object.height = height;
		object.native = texture;
		_animateGroup->textures.push_back(object);

		if (data)
		free(data);
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	bool AnimateLoader::CreateEmptyConfig(AnimateConfig* _config)
	{
		_config->type = 1;
		_config->layers = 1;
		_config->duration = 1;
		_config->width = SMALL_IMAGE_WIDTH;
		_config->height = SMALL_IMAGE_HEIGHT;
		_config->offset_x = 0;
		_config->offset_y = 0;
		_config->ref_point = 0;
		_config->ref_eye_dist = 100;
		_config->ref_face_size = 100;
		LOGD("load default config ok");
		return true;
	}

	bool AnimateLoader::CreateEmptyImage(AnimateGroup* _animateGroup)
	{
		// a empty texture
		GLuint native;
		glGenTextures(1, &native);
		glBindTexture(GL_TEXTURE_2D, native);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SMALL_IMAGE_WIDTH, SMALL_IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		AnimateTexture object;
		object.width = SMALL_IMAGE_WIDTH;
		object.height = SMALL_IMAGE_HEIGHT;
		object.native = native;
		_animateGroup->textures.push_back(object);

		glBindTexture(GL_TEXTURE_2D, 0);
		LOGD("load default animate ok");
		return true;
	}
}
