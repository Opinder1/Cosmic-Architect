#include "World.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	World::World()
	{}

	World::~World()
	{}

	void World::Reset()
	{
		m_world.reset();
	}

	void World::StartRest(uint16_t port, bool monitor)
	{
		m_world.set(flecs::Rest{ port, nullptr, nullptr });

		if (monitor)
		{
			m_world.import<flecs::monitor>();
		}
	}

	void World::SetThreads(int64_t threads)
	{
		m_world.set_threads(threads);
	}

	bool World::Progress(double delta)
	{
		return m_world.progress(delta);
	}

	void World::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("reset"), &World::Reset);
		godot::ClassDB::bind_method(godot::D_METHOD("start_rest", "port", "monitor"), &World::StartRest, 27750, true);
		godot::ClassDB::bind_method(godot::D_METHOD("set_threads", "thread_count"), &World::SetThreads);
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &World::Progress, 0.0);
	}

	WorldNode::WorldNode() :
		m_owner(false)
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
			m_world->Progress(delta);
		}
	}

	void WorldNode::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_world", "world"), &WorldNode::SetWorld);
		godot::ClassDB::bind_method(godot::D_METHOD("get_world"), &WorldNode::GetWorld);
		ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "world", godot::PROPERTY_HINT_RESOURCE_TYPE, "World"), "set_world", "get_world");
	}
}