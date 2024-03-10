#include "World.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_world
{
	World::World() :
		m_info(m_world.get_info())
	{}

	World::~World()
	{}

	void World::StartRestServer(uint64_t port, bool monitor)
	{
		if (port >= UINT16_MAX)
		{
			DEBUG_PRINT_ERROR("Port should be 65535 or less");
			return;
		}

		m_world.set(flecs::Rest{ static_cast<uint16_t>(port), nullptr, nullptr });

		if (monitor)
		{
			m_world.import<flecs::monitor>();
		}
	}

	void World::StopRestServer()
	{
		m_world.remove<flecs::Rest>();
	}

	bool World::HasRestServer()
	{
		return m_world.has<flecs::Rest>();
	}

	void World::SetThreads(uint64_t threads)
	{
		if (threads > INT32_MAX)
		{
			DEBUG_PRINT_ERROR("Thread count should be 2^31 or less");
			return;
		}

		m_world.set_threads(static_cast<int32_t>(threads));
	}

	void World::PreallocateForEntities(uint64_t entity_count)
	{
		if (entity_count > INT32_MAX)
		{
			DEBUG_PRINT_ERROR("Entity count count should be 2^31 or less");
			return;
		}

		m_world.dim(static_cast<int32_t>(entity_count));
	}

	void World::SetEntityRange(uint64_t min_id, uint64_t max_id)
	{
		m_world.set_entity_range(min_id, max_id);
	}

	void World::SetEntityRangeCheck(bool enabled)
	{
		m_world.enable_range_check(enabled);
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

	void World::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_rest_server", "port", "monitor"), &World::StartRestServer, 27750, true);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_rest_server"), &World::StopRestServer);
		godot::ClassDB::bind_method(godot::D_METHOD("has_rest_server"), &World::HasRestServer);
		godot::ClassDB::bind_method(godot::D_METHOD("set_threads", "thread_count"), &World::SetThreads);
		godot::ClassDB::bind_method(godot::D_METHOD("preallocate_for_entities", "entity_count"), &World::PreallocateForEntities);
		godot::ClassDB::bind_method(godot::D_METHOD("set_entity_range", "min_id", "max_id"), &World::SetEntityRange);
		godot::ClassDB::bind_method(godot::D_METHOD("set_entity_range_check", "enabled"), &World::SetEntityRangeCheck);
		godot::ClassDB::bind_method(godot::D_METHOD("reset"), &World::Reset);
		godot::ClassDB::bind_method(godot::D_METHOD("reset_time"), &World::ResetTime);
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &World::Progress, 0.0);
	}
}