#pragma once

#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

// A uuid that can be used in godot functions though will appear as a color
using UUID = godot::Color;
using UUIDVector = godot::PackedColorArray;

struct UUIDHash
{
	size_t operator()(const UUID&) const;
};