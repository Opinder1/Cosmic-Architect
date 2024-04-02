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

		world.component<SpatialEntity3DTag>().add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialWorld3DComponent>();
		world.component<SpatialPosition3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialWorld3DThread>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialBox3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialSphere3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);

		world.system<SpatialWorld3DComponent>("SpatialWorldProgressSystem")
			.multi_threaded()
			.each([](flecs::iter& iter, size_t index, SpatialWorld3DComponent& spatial_world)
		{

		});

		world.system<SpatialWorld3DThread>("SpatialWorldThreadProgressSystem")
			.multi_threaded()
			.term<const SpatialWorld3DComponent>().parent()
			.each([](flecs::iter& it, size_t index, SpatialWorld3DThread& spatial_world_thread)
		{
			const SpatialWorld3DComponent& spatial_world = *it.field<const SpatialWorld3DComponent>(2);

			spatial_world_thread.aabb = spatial_world.aabb;
		});
	}
}