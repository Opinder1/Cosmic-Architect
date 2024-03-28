#include "PerWorker.h"
#include "Components.h"
#include "Voxel.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	// The component that each thread will have one instance of
	struct PerThreadComponent
	{
		size_t id = 0;
	};

	PerWorkerModule::PerWorkerModule(flecs::world& world)
	{
		world.module<PerWorkerModule>("PerWorkerModule");

		// Create an entity with the component for each thread
		for (size_t i = 0; i < world.get_threads(); i++)
		{
			world.entity().emplace<PerThreadComponent>(i);
		}

		world.system<PerThreadComponent>()
			.multi_threaded()
			.iter([](flecs::iter& iter, PerThreadComponent* per_thread)
		{
			for (size_t index : iter)
			{
				size_t component_id = per_thread[index].id;
				size_t thread_id = godot::OS::get_singleton()->get_thread_caller_id();
				size_t component_address = (size_t)per_thread;
				godot::UtilityFunctions::print("C:", component_id, " T:", thread_id, " A:", component_address);
			}
		});
	}
}