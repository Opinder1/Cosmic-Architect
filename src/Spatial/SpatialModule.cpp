#include "SpatialModule.h"

#include "Spatial.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>();

		world.component<SpatialEntityTag>();
		world.component<SpatialPositionComponent>();
		world.component<SpatialBoxComponent>();
		world.component<SpatialSphereComponent>();
		world.component<SpatialCameraComponent>();
		world.component<SpatialWorldComponent>();
		world.component<SpatialWorldThreadComponent>();

		world.system<SpatialWorldThreadComponent>("SpatialWorldLoaderSystem")
			.multi_threaded()
			.each(&SpatialModule::ProcessThread);

		world.system<SpatialWorldComponent>("SpatialWorldFlushCommandsSystem")
			.multi_threaded()
			.each(&SpatialModule::ProcessWorld);
	}
	
	void SpatialModule::ProcessThread(flecs::entity entity, SpatialWorldThreadComponent& world_thread)
	{
		auto& threads = world_thread.world->threads;
		auto& scales = world_thread.world->scales;

		SpatialWorld3DThread& thread = threads[world_thread.id];

		for (auto&& loader : thread.loaders_to_update)
		{
			for (auto&& scale : scales)
			{
				// Make loader load and unload
			}
		}
	}

	void SpatialModule::ProcessWorld(flecs::entity entity, SpatialWorldComponent& spatial_world)
	{
		Clock::time_point now = Clock::now();

		auto& threads = spatial_world.world->threads;
		auto& scales = spatial_world.world->scales;

		for (auto&& [id, thread] : threads)
		{
			for (size_t scale = 0; scale < k_max_world_scale; scale++)
			{
				auto& nodes = scales[scale];
				auto& lists = thread.scale_lists[scale];

				for (auto&& [pos, node] : lists.nodes_add)
				{
					nodes.emplace(pos, node);
				}

				lists.nodes_add.clear();

				for (godot::Vector3i pos : lists.nodes_changed)
				{
					auto it = nodes.find(pos);

					if (it == nodes.end())
					{
						continue;
					}

					// Process(it->second);
				}

				lists.nodes_changed.clear();

				for (godot::Vector3i pos : lists.nodes_no_loaders)
				{
					if (now - nodes[pos]->last_update_time > 5s)
					{
						lists.nodes_remove.push_back(pos);
					}
				}

				lists.nodes_no_loaders.clear();

				for (godot::Vector3i pos : lists.nodes_remove)
				{
					nodes.erase(pos);
				}

				lists.nodes_remove.clear();
			}
		}
	}
}