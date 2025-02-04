#ifndef _FNV1A_H_
#define _FNV1A_H_

#include <stddef.h> 
#include <stdint.h> 

uint64_t fnv1a(void *data, size_t len)
{
    uint64_t FNV_prime = 0x00000100000001b3;
    uint64_t hash      = 0xcbf29ce484222325;

    for (size_t i = 0; i < len; i++) {
        hash ^= *((uint8_t *) data + i);
        hash *= FNV_prime;
    }

    return hash;
}

#endif
