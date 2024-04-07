#include "Hash.h"

size_t VectorHash::operator()(const godot::Vector3i& vec) const noexcept
{
    return robin_hood::hash_bytes(&vec, sizeof(vec));
}