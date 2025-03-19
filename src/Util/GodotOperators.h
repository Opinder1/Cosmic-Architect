#pragma once

#include <godot_cpp/variant/vector3i.hpp>

namespace godot
{
    Vector3i operator>>(Vector3i vec, int32_t val);

    Vector3i operator<<(Vector3i vec, int32_t val);
};