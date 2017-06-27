#include "include/GPUImageEnvironment.h"
#include "include/GPUImageBase.h"

namespace e
{
    const char* const kGPUImagePathDefault = "/storage/emulated/0/hivideo/";

    inline bool CreateDirectory(const char* path)
    {
        struct stat t;
        if (stat(path, &t)==0){
            return S_ISDIR(t.st_mode);
        }else{
            return mkdir(path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)==0;
        }
    }

    bool MakeSureDirectoryPathExists(const char* path)
    {
        if (!path) return false;
        const char* s = path;
        const char* e = s + strlen(path);
        while(isspace(*s) && s<e)s++;
        while(isspace(*e) && s<e)e--;

        while(s < e)
        {
            if ((*s=='/' || *s=='\\') && (s - path > 0))
            {
                std::string sub = std::string(path, (size_t)(s - path));
                if (!CreateDirectory(sub.c_str())){
                    LOGE("create directory failed:%s", sub.c_str());
                    return false;
                }
            }
            s++;
        }
        e--;
        return (*e=='/' || *e=='\\')?true:CreateDirectory(path);
    }

    string GPUImageEnvironment::paths[__KG_MAX_COUNT__];
    void GPUImageEnvironment::SetPath(const PathType type, const string path)
    {
        paths[type] = path;
        MakeSureDirectoryPathExists(path.c_str());
    }
    
    string GPUImageEnvironment::GetPath(const PathType type)
    {
        return paths[type];
    }
    
}