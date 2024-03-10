#pragma once

#include <flecs/flecs.h>

#include <godot_cpp/classes/resource.hpp>

namespace voxel_world
{
	class World : public godot::Resource
	{
		GDCLASS(World, godot::Resource);

	public:
		World();
		~World();

		void StartRestServer(uint64_t port, bool monitor);

		void StopRestServer();

		bool HasRestServer();

		void SetThreads(uint64_t threads);

		void PreallocateForEntities(uint64_t entity_count);

		void SetEntityRange(uint64_t min_id, uint64_t max_id);

		void SetEntityRangeCheck(bool enabled);

		void Reset();

		void ResetTime();

		bool Progress(double delta);

	protected:
		static void _bind_methods();

	private:
		flecs::world m_world;
		const flecs::world_info_t* m_info;
	};
}