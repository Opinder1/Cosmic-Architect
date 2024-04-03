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
		godot::Vector3i start = pos - godot::Vector3i(radius, radius, radius);
		godot::Vector3i end = pos + godot::Vector3i(radius, radius, radius);
		for (int32_t x = start.x; x < end.x; x++)
		{
			for (int32_t y = start.x; y < end.x; y++)
			{
				for (int32_t z = start.x; z < end.x; z++)
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

		world.add<WorldTime>();

		world.component<SpatialEntity3DTag>().add_second<SpatialWorld3DComponent>(flecs::OneOf);

		world.component<SpatialWorld3DComponent>();
		world.component<SpatialPosition3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialScaleThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialRegionThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialNodeThread3DComponent>().add_second<SpatialWorld3DComponent>(flecs::OneOf);
		world.component<SpatialBox3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialSphere3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);
		world.component<SpatialLoader3DComponent>().add_second<SpatialPosition3DComponent>(flecs::With);

		world.entity<WorldNodeProgressPhase>().add(flecs::Phase).depends_on(flecs::OnUpdate);
		world.entity<WorldRegionProgressPhase>().add(flecs::Phase).depends_on<WorldNodeProgressPhase>();
		world.entity<WorldScaleProgressPhase>().add(flecs::Phase).depends_on<WorldRegionProgressPhase>();
		world.entity<WorldProgressPhase>().add(flecs::Phase).depends_on<WorldScaleProgressPhase>();

		world.system("WorldNodeProgressSync").kind<WorldNodeProgressPhase>().iter([](flecs::iter& it) {});
		world.system("WorldRegionProgressSync").kind<WorldRegionProgressPhase>().iter([](flecs::iter& it) {});
		world.system("WorldScaleProgressSync").kind<WorldScaleProgressPhase>().iter([](flecs::iter& it) {});
		world.system("WorldProgressSync").kind<WorldProgressPhase>().iter([](flecs::iter& it) {});

		world.system<WorldTime>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(1).src<WorldTime>()
			.iter([](flecs::iter& it, WorldTime* world_time)
		{
			world_time->frame_start = Clock::now();
		});

		world.system<SpatialLoader3DComponent, const SpatialWorld3DComponent, const WorldTime>("SpatialWorldLoaderUpdateNodes")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.term_at(2).parent()
			.term_at(3).src<WorldTime>()
			.each([](flecs::iter& iter, size_t, SpatialLoader3DComponent& spatial_loader, const SpatialWorld3DComponent& world, const WorldTime& world_time)
		{
			for (size_t scale_index = spatial_loader.min_lod; scale_index < spatial_loader.max_lod; scale_index++)
			{
				const SpatialScale3D& spatial_scale = world.world->scales[scale_index];

				ForEachCoordInSphere(spatial_loader.coord.pos, spatial_loader.dist_per_lod, [scale_index, &spatial_scale, &world_time](godot::Vector3i pos)
				{
					auto it = spatial_scale.nodes.find(pos);

					if (it != spatial_scale.nodes.end())
					{
						it->second->last_update_time = world_time.frame_start;
					}
				});
			}
		});

		world.system<SpatialWorld3DComponent>("SpatialWorldProgress")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each([](SpatialWorld3DComponent& spatial_world)
		{
			for (SpatialScale3D& scale : spatial_world.world->scales)
			{

			}
		});

		world.system<SpatialScaleThread3DComponent>("SpatialWorldScaleProgress")
			.multi_threaded()
			.kind<WorldScaleProgressPhase>()
			.each([](SpatialScaleThread3DComponent& spatial_world_scale)
		{
			SpatialWorld3D* spatial_world = spatial_world_scale.world;

			SpatialScale3D& scale = spatial_world->scales[spatial_world_scale.scale];
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