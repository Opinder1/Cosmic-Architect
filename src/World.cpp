#include "World.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	World::World()
	{}

	World::~World()
	{}

	void World::StartRest(uint64_t port, bool monitor)
	{
		if (port > UINT16_MAX)
		{
			// Error
		}

		m_world.set(flecs::Rest{ static_cast<uint16_t>(port), nullptr, nullptr });

		if (monitor)
		{
			m_world.import<flecs::monitor>();
		}
	}

	void World::SetThreads(uint64_t threads)
	{
		if (threads > INT32_MAX)
		{
			// Error
		}

		m_world.set_threads(static_cast<int32_t>(threads));
	}

	void World::Reset()
	{
		m_world.reset();
	}

	void World::ResetTime()
	{
		m_world.reset_clock();
	}

	bool World::Progress(double delta)
	{
		return m_world.progress(delta);
	}

	float World::GetDeltaTime()
	{
		return m_world.delta_time();
	}

	void World::PreallocateForEntities(uint64_t entity_count)
	{
		if (entity_count > INT32_MAX)
		{
			// Error
		}

		m_world.dim(static_cast<int32_t>(entity_count));
	}

	void World::SetEntityRange(uint64_t min, uint64_t max)
	{
		m_world.set_entity_range(min, max);
	}

	void World::SetEntityRangeCheck(bool enabled)
	{
		m_world.enable_range_check(enabled);
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