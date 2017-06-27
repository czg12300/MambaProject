#ifndef E_HASHKEY_H
#define E_HASHKEY_H
#include <stdlib.h>

namespace e
{
    typedef size_t key_t;

    key_t GenHashKey(const char* text, int length);
}

#endif