#include "World.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	World::World()
	{
		m_world.set_threads(12);
	}

	World::~World()
	{}

	bool World::progress(double delta)
	{
		return m_world.progress(delta);
	}

	void World::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &World::progress);
	}

	WorldNode::WorldNode()
	{}

	WorldNode::~WorldNode()
	{}

	void WorldNode::SetWorld(const godot::Ref<World>& world)
	{
		m_world = world;
	}

	godot::Ref<World> WorldNode::GetWorld()
	{
		return m_world;
	}

	void WorldNode::_process(double delta)
	{
		if (m_world.is_valid())
		{
			m_world->progress(delta);
		}
	}

	void WorldNode::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_world", "world"), &WorldNode::SetWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("get_world"), &WorldNode::GetWorld);
		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "world", godot::PROPERTY_HINT_RESOURCE_TYPE, "World"), "set_world", "get_world");
	}
}