#include "SpatialModule.h"
#include "Spatial.h"

#include "Util/Debug.h"

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
		world.module<SpatialModule>("SpatialModule");

		world.component<SpatialEntity3DTag>().add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialWorld3DComponent>();
		world.component<SpatialPosition3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialWorld3DThreadComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialBox3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialSphere3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);

		world.system<SpatialWorld3DComponent>("SpatialWorldProgressSystem")
			.multi_threaded()
			.iter([](flecs::iter& it, SpatialWorld3DComponent* spatial_worlds)
		{

		});

		world.system<SpatialWorld3DComponent>("SpatialWorldDelegateToThreadsSystem")
			.multi_threaded()
			.iter([](flecs::iter& it, SpatialWorld3DComponent* spatial_worlds)
		{
			for (size_t i : it)
			{
				it.world().filter_builder<SpatialWorld3DThreadComponent>()
					.term(flecs::ChildOf, it.entity(i))
					.each([&spatial_world = spatial_worlds[i]](flecs::entity entity, SpatialWorld3DThreadComponent& spatial_world_thread)
				{
					spatial_world.region = spatial_world_thread.region;
				});
			}
		});

		world.system<SpatialWorld3DThreadComponent>("SpatialWorldThreadProgressSystem")
			.term<const SpatialWorld3DComponent>().parent()
			.multi_threaded()
			.iter([](flecs::iter& it, SpatialWorld3DThreadComponent* spatial_world_threads)
		{
			auto spatial_worlds = it.field<const SpatialWorld3DComponent>(2);

			for (size_t i : it)
			{
				spatial_world_threads[i].region = spatial_worlds[i].region;
			}
		});
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3DComponent& world, SpatialCoord3D coord)
	{
		DEBUG_ASSERT(coord.scale < k_max_world_scale, "The coordinates scale is out of range");

		auto& nodes = world.scales[coord.scale].nodes;

		auto it = nodes.find(coord.pos);

		if (it == nodes.end())
		{
			return nullptr;
		}

		return it->second;
	}
}