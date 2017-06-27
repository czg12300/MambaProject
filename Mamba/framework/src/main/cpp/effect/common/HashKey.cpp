//
//HashKey.cpp
//
#include "include/HashKey.h"
namespace e
{
	key_t GenHashKey(const char* text, int length)
    {
        key_t h = 0, g;
        const char *end = text + length;

        while (text < end) {
            h = (h << 4) + *text++;
            if ((g = (h & 0xf0000000))) {
                h = h ^ (g >> 24);
                h = h ^ g;
            }
        }
        
        return h;
    }	
}