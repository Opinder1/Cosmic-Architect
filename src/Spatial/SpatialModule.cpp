#include "SpatialModule.h"

#include "Spatial.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	template<class Callable>
	void ForEachCoordInSphere(godot::Vector3 pos, double radius, Callable callable)
	{
		godot::Vector3 start = pos - godot::Vector3(radius, radius, radius);
		godot::Vector3 end = pos + godot::Vector3(radius, radius, radius);
		for (double x = start.x; x < end.x; x++)
		{
			for (double y = start.x; y < end.x; y++)
			{
				for (double z = start.x; z < end.x; z++)
				{
					godot::Vector3i voxel_pos{ x, y, z };

					if (pos.distance_squared_to(voxel_pos) < radius)
					{
						callable(voxel_pos);
					}
				}
			}
		}
	}

	SpatialModule::SpatialModule(flecs::world& world)
	{
		world.module<SpatialModule>();

		world.component<SpatialEntityTag>().add_second<SpatialWorldComponent>(flecs::OneOf);

		world.component<SpatialPositionComponent>().add_second<SpatialWorldComponent>(flecs::OneOf);
		world.component<SpatialBoxComponent>().add_second<SpatialPositionComponent>(flecs::With);
		world.component<SpatialSphereComponent>().add_second<SpatialPositionComponent>(flecs::With);
		world.component<SpatialCameraComponent>().add_second<SpatialPositionComponent>(flecs::With);
		world.component<SpatialWorldComponent>();
		world.component<SpatialWorldThreadComponent>().add_second<SpatialWorldComponent>(flecs::OneOf);

		world.system<SpatialWorldThreadComponent>("SpatialWorldProgressSystem")
			.no_readonly()
			.multi_threaded()
			.each(&SpatialModule::ProgressWorldThread);

		world.system<SpatialWorldComponent>("SpatialWorldThreadProgressSystem")
			.no_readonly()
			.multi_threaded()
			.each(&SpatialModule::ProgressWorld);

		auto world_entity_prefab = world.prefab("WorldEntity");

		world_entity_prefab.add<SpatialPositionComponent>();
	}

	void AddEntityToWorld(flecs::entity entity, flecs::entity spatial_world, uint32_t scale)
	{
		entity.add<SpatialEntityTag>(spatial_world.id());
	}
	
	void SpatialModule::ProgressWorldThread(flecs::entity entity, SpatialWorldThreadComponent& world_thread)
	{
		Clock::time_point now = Clock::now();

		auto& threads = world_thread.world->threads;
		auto& scales = world_thread.world->scales;

		SpatialWorld3DThread& thread = threads[world_thread.id];

		for (auto&& loader : thread.loaders_to_update)
		{
			for (size_t scale_index = loader->min_lod; scale_index < loader->max_lod; scale_index++)
			{
				auto& scale = scales[scale_index];

				ForEachCoordInSphere(loader->coord.pos, loader->dist_per_lod, [&scale, now](godot::Vector3i pos)
				{
					scale.nodes[pos]->last_update_time = now;
				});
			}
		}
	}

	void SpatialModule::ProgressWorld(flecs::entity entity, SpatialWorldComponent& spatial_world)
	{
		Clock::time_point now = Clock::now();

		auto& threads = spatial_world.world->threads;
		auto& scales = spatial_world.world->scales;

		for (auto&& [id, thread] : threads)
		{
			for (size_t scale = 0; scale < k_max_world_scale; scale++)
			{
				auto& nodes = scales[scale].nodes;
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