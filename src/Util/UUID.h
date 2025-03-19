#pragma once

#include <godot_cpp/variant/vector4i.hpp>
#include <godot_cpp/variant/array.hpp>

// A uuid that can be used in godot functions though will appear as a color
using UUID = godot::Vector4i;
using UUIDVector = godot::Array;

struct UUIDHash
{
    size_t operator()(const UUID&) const;
};