#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

// Macro shortcuts for creating more complex property types

godot::PropertyInfo MakeObjectProperty(const godot::StringName& name, const godot::String& object_string);
godot::PropertyInfo MakeEnumProperty(const godot::StringName& name, const godot::String& enum_string);

#define RID_PROPERTY(name) godot::PropertyInfo(godot::Variant::RID, name)
#define OBJECT_PROPERTY(name, object_) MakeObjectProperty(name, #object_)
#define ENUM_PROPERTY(name, enum_) MakeEnumProperty(name, #enum_)