#pragma once

#include <godot_cpp/variant/vector3i.hpp>

#include <robin_hood/robin_hood.h>

template<class T>
struct ByteHash
{
    size_t operator()(const T& vec) const noexcept
    {
        return robin_hood::hash_bytes(&vec, sizeof(T));
    }
};

struct VectorHash
{
    size_t operator()(const godot::Vector3i& vec) const noexcept;
};