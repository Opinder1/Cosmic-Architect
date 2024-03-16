#pragma once

#include <flecs/flecs.h>

#include <godot_cpp/classes/resource.hpp>

namespace voxel_game
{
	class FlecsWorld : public godot::RefCounted
	{
		GDCLASS(FlecsWorld, godot::RefCounted);

	public:
		FlecsWorld();
		~FlecsWorld();

		void StartRestServer(uint64_t port, bool monitor);
		void StopRestServer();
		bool RunningRestServer();

		void SetThreads(uint64_t threads);

		void SetEntityRange(uint64_t min_id, uint64_t max_id);

		void ResetTime();
		void SetTimeMulti(double multi);
		bool Progress(double delta);

	protected:
		static void _bind_methods();

	protected:
		flecs::world m_world;
		const flecs::world_info_t* m_info;
	};
}