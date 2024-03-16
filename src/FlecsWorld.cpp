#include "FlecsWorld.h"

#include "Util/Debug.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	FlecsWorld::FlecsWorld() :
		m_info(m_world.get_info())
	{}

	FlecsWorld::~FlecsWorld()
	{}

	void FlecsWorld::StartRestServer(uint64_t port, bool monitor)
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

		DEBUG_PRINT_INFO("Started rest server on port " + godot::UtilityFunctions::str(port) + " with monitoring " + (monitor ? "enabled" : "disabled"));
	}

	void FlecsWorld::StopRestServer()
	{
		m_world.remove<flecs::Rest>();
	}

	bool FlecsWorld::RunningRestServer()
	{
		return m_world.has<flecs::Rest>();
	}

	void FlecsWorld::SetThreads(uint64_t threads)
	{
		if (threads > INT32_MAX)
		{
			DEBUG_PRINT_ERROR("Thread count should be 2^31 or less");
			return;
		}

		m_world.set_threads(static_cast<int32_t>(threads));
	}

	void FlecsWorld::SetEntityRange(uint64_t min_id, uint64_t max_id)
	{
		m_world.set_entity_range(min_id, max_id);
		m_world.enable_range_check(true);
	}

	void FlecsWorld::ResetTime()
	{
		m_world.reset_clock();
	}

	void FlecsWorld::SetTimeMulti(double multi)
	{
		m_world.set_time_scale(multi);
	}

	bool FlecsWorld::Progress(double delta)
	{
		return m_world.progress(delta);
	}

	void FlecsWorld::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("start_rest_server", "port", "monitor"), &FlecsWorld::StartRestServer, 27750, true);
		godot::ClassDB::bind_method(godot::D_METHOD("stop_rest_server"), &FlecsWorld::StopRestServer);
		godot::ClassDB::bind_method(godot::D_METHOD("running_rest_server"), &FlecsWorld::RunningRestServer);
		godot::ClassDB::bind_method(godot::D_METHOD("set_threads", "thread_count"), &FlecsWorld::SetThreads);
		godot::ClassDB::bind_method(godot::D_METHOD("set_entity_range", "min_id", "max_id"), &FlecsWorld::SetEntityRange);
		godot::ClassDB::bind_method(godot::D_METHOD("set_time_multi", "enabled"), &FlecsWorld::SetTimeMulti);
		godot::ClassDB::bind_method(godot::D_METHOD("reset_time"), &FlecsWorld::ResetTime);
		godot::ClassDB::bind_method(godot::D_METHOD("progress", "delta"), &FlecsWorld::Progress, 0.0);
	}
}