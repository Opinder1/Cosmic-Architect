#pragma once

#include <robin_hood/robin_hood.h>

template<class T>
struct ByteHash
{
    size_t operator()(const T& vec) const noexcept
    {
        return robin_hood::hash_bytes(&vec, sizeof(T));
    }
};