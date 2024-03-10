#include "WorldNode.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	WorldNode::WorldNode() :
		m_owner(false)
	{}

	WorldNode::~WorldNode()
	{}

	void WorldNode::SetWorld(const godot::Ref<World>& world)
	{
		m_world = world;
		m_owner = false;
	}

	godot::Ref<World> WorldNode::GetWorld()
	{
		return m_world;
	}

	void WorldNode::LoadWorld(const godot::String& path)
	{
		m_world.instantiate();
		m_owner = true;

		m_world->set_path(path);
	}

	void WorldNode::_process(double delta)
	{
		if (m_owner && m_world.is_valid())
		{
			m_world->Progress(delta);
		}
	}

	void WorldNode::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_world", "world"), &WorldNode::SetWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("get_world"), &WorldNode::GetWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("load_world", "path"), &WorldNode::LoadWorld);
		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "world", godot::PROPERTY_HINT_RESOURCE_TYPE, "World"), "set_world", "get_world");
	}
}