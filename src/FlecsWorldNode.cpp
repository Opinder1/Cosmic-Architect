#include "FlecsWorldNode.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	FlecsWorldNode::FlecsWorldNode()
	{}

	FlecsWorldNode::~FlecsWorldNode()
	{}

	void FlecsWorldNode::SetWorld(const godot::Ref<FlecsWorld>& world)
	{
		m_world = world;
	}

	godot::Ref<FlecsWorld> FlecsWorldNode::GetWorld()
	{
		return m_world;
	}

	void FlecsWorldNode::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_world", "world"), &FlecsWorldNode::SetWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("get_world"), &FlecsWorldNode::GetWorld);
		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "world", godot::PROPERTY_HINT_NONE, "world", godot::PROPERTY_USAGE_NONE, "world"), "set_world", "get_world");
	}
}