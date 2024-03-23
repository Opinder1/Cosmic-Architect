#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

godot::PropertyInfo MakeObjectProperty(const godot::StringName& name, const godot::String& object_string);
godot::PropertyInfo MakeEnumProperty(const godot::StringName& name, const godot::String& enum_string);

#define OBJECT_PROPERTY(name, object_) MakeObjectProperty(name, #object_)
#define ENUM_PROPERTY(name, enum_) MakeEnumProperty(name, #enum_)