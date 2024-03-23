#include "PropertyMacros.h"

#include <godot_cpp/core/type_info.hpp>

godot::PropertyInfo MakeObjectProperty(const godot::StringName& name, const godot::String& object_string)
{
	godot::PropertyInfo info;

	info.type = godot::Variant::OBJECT;
	info.name = name;
	info.class_name = object_string;
	info.hint_string = object_string;

	return info;
}

godot::PropertyInfo MakeEnumProperty(const godot::StringName& name, const godot::String& enum_string)
{
	godot::PropertyInfo info;

	info.type = godot::Variant::INT;
	info.name = name;
	info.class_name = godot::String();
	info.hint = godot::PROPERTY_HINT_NONE;
	info.hint_string = godot::enum_qualified_name_to_class_info_name(enum_string);
	info.usage = godot::PROPERTY_USAGE_DEFAULT | godot::PROPERTY_USAGE_CLASS_IS_ENUM;

	return info;
}