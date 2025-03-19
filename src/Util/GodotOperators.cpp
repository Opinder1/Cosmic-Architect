#include "GodotOperators.h"

namespace godot
{
    Vector3i operator>>(Vector3i vec, int32_t val)
    {
        return Vector3i{ vec.x >> val, vec.y >> val, vec.z >> val };
    }

    Vector3i operator<<(Vector3i vec, int32_t val)
    {
        return Vector3i{ vec.x << val, vec.y << val, vec.z << val };
    }
}