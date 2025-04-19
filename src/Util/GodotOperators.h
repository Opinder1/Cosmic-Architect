#pragma once

#include <godot_cpp/variant/vector3i.hpp>

// Useful operators that haven't already been added for godot types

namespace godot
{
    Vector3i operator>>(Vector3i vec, int32_t val);

    Vector3i operator<<(Vector3i vec, int32_t val);
};