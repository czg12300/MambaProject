//
// Created by liyongfa on 2016/4/9.
//

#include "include/GPUImageContext.h"
#include "include/GPUImageEnvironment.h"
#include <GLES/gl.h>

namespace e
{
	static string trim(string s)
	{
		if (s.empty())
		{
			return s;
		}

		s.erase(0, s.find_first_not_of(" "));
		s.erase(s.find_last_not_of(" ") + 1);
		return s;
	}

	static size_t split(string & s, string c, vector<string> & results)
	{
		size_t last = 0;
		size_t index = s.find_first_of(c, last);
		while (index != std::string::npos)
		{
			results.push_back(s.substr(last, index-last));
			last = index+1;
			index=s.find_first_of(c,last);
		}
		if (index-last>0)
		{
			string a = trim(s.substr(last, index - last));
			if(!a.empty()) results.push_back(a);
		}

		return results.size();
	}

    GLContext::GLContext(void)
    {
        const char* version = (const char*)glGetString(GL_VERSION);
        LOGD("OpenGL ES version: %s", version?version:"unknown");

		vector<string> items;
		string ext = (const char*)glGetString(GL_EXTENSIONS);
		split(ext, string(" "), items);

		LOGD("OpenGL ES extensions:");
		for (size_t i=0; i<items.size(); i++)
		{
			LOGD("[%02zd]\t%s", i, items[i].c_str());
		}

        GPUImageEnvironment::SetPath(KG_PATH_ROOT, "/storage/emulated/0/HiVideo/");
        GPUImageEnvironment::SetPath(KG_PATH_DATA, "/storage/emulated/0/HiVideo/data/");
        GPUImageEnvironment::SetPath(KG_PATH_EFFECT, "/storage/emulated/0/HiVideo/effect/");
        GPUImageEnvironment::SetPath(KG_PATH_OUTPUT, "/storage/emulated/0/HiVideo/output/");
        GPUImageEnvironment::SetPath(KG_PATH_LOG, "/storage/emulated/0/HiVideo/log/");
    }
    
    GLContext::~GLContext(void)
    {

    }
}
