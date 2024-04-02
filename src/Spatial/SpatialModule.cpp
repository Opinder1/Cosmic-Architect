#include "SpatialModule.h"
#include "SpatialComponents.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/os.hpp>

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
		world.component<SpatialScaleThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialRegionThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialNodeThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialBox3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialSphere3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);

		world.system<SpatialWorld3DComponent>("SpatialWorldProgress")
			.multi_threaded()
			.iter([](flecs::iter& it, SpatialWorld3DComponent* spatial_worlds)
		{
			for (size_t i : it)
			{ 
				for (auto& scale : spatial_worlds[i].world->scales)
				{
					if (scale.m.try_lock())
					{
						//std::this_thread::yield();
						scale.m.unlock();
					}
					else
					{
						DEBUG_PRINT_ERROR("Should have locked mutex");
					}
				}

				it.entity(i).modified<SpatialWorld3DComponent>();
			}
		});

		world.system<SpatialScaleThread3DComponent>("SpatialWorldScaleProgress")
			.term<SpatialWorld3DComponent>().parent().inout()
			.multi_threaded()
			.iter([](flecs::iter& it, SpatialScaleThread3DComponent* spatial_world_scales)
		{
			for (size_t i : it)
			{ 
				SpatialWorld3D& spatial_world = *spatial_world_scales[i].world;

				SpatialScale3D& scale = spatial_world.scales[spatial_world_scales[i].scale];

				if (scale.m.try_lock())
				{
					//std::this_thread::yield();
					scale.m.unlock();
				}
				else
				{
					DEBUG_PRINT_ERROR("Should have locked mutex");
				}

				it.entity(i).parent().modified<SpatialWorld3DComponent>();
			}
		});
	}

	SpatialNode3D* SpatialModule::GetNode(const SpatialWorld3D& world, SpatialCoord3D coord)
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