#ifndef _E_GPUIMAGE_CONFIG_H_
#define _E_GPUIMAGE_CONFIG_H_
#include <string.h>
#include <string>
using namespace std;

namespace e
{
    extern const char* const kGPUImagePathDefault;
    
    typedef enum{
        KG_PATH_ROOT,
        KG_PATH_DATA,
        KG_PATH_EFFECT,
        KG_PATH_OUTPUT,
        KG_PATH_LOG,
        __KG_MAX_COUNT__
    }PathType;
    
    class GPUImageEnvironment
    {
    public:
        static void SetPath(const PathType type, const string path);
        static string GetPath(const PathType type);
    protected:
        static string paths[__KG_MAX_COUNT__];
    };
}
#endif