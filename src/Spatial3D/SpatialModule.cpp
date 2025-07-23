#include "SpatialModule.h"
#include "SpatialTraverse.h"

#include "Components.h"
#include "UniverseSimulation.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	void OnLoadSpatialEntity(Simulation& simulation, entity::WRef entity)
	{
		WorldPtr world = entity->*&CWorld::world;

		SpatialTypeData& type = *static_cast<SpatialTypeData*>(world->*&World::type);

		simulation.spatial_worlds.push_back(world);

		type.worlds.push_back(world);

		WorldForEachScale(world, [&](ScalePtr scale)
		{
			simulation.spatial_scales.push_back(scale);

			type.scales.push_back(scale);
		});
	}

	void OnUnloadSpatialEntity(Simulation& simulation, entity::WRef entity)
	{
		UnloadWorld(entity->*&CWorld::world);
	}

	void OnLoadLoaderEntity(Simulation& simulation, entity::WRef entity)
	{
		((entity->*&CEntity::world)->*&PartialWorld::loaders).push_back(entity);
	}

	void OnUnloadLoaderEntity(Simulation& simulation, entity::WRef entity)
	{
		unordered_erase((entity->*&CEntity::world)->*&PartialWorld::loaders, entity::Ref(entity));
	}

	void Initialize(Simulation& simulation)
	{
		simulation.entity_factory.AddCallback<CWorld>(PolyEvent::BeginLoad, cb::BindArg<OnLoadSpatialEntity>(simulation));
		simulation.entity_factory.AddCallback<CWorld>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadSpatialEntity>(simulation));
		simulation.entity_factory.AddCallback<CLoader>(PolyEvent::BeginLoad, cb::BindArg<OnLoadLoaderEntity>(simulation));
		simulation.entity_factory.AddCallback<CLoader>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadLoaderEntity>(simulation));
	}

	void Uninitialize(Simulation& simulation)
	{
		DEBUG_ASSERT(simulation.spatial_worlds.empty(), "All worlds should have been destroyed");
		DEBUG_ASSERT(simulation.spatial_scales.empty(), "All scales should have been destroyed");
	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return simulation.spatial_worlds.empty() && simulation.spatial_scales.empty();
	}

	void Update(Simulation& simulation)
	{
		// Remove worlds that have finished unloading
		for (auto it = simulation.spatial_worlds.begin(); it != simulation.spatial_worlds.end();)
		{
			WorldPtr world = *it;

			if (IsWorldUnloading(world) && WorldGetNodeCount(world) == 0)
			{
				SpatialTypeData& type = *static_cast<SpatialTypeData*>(world->*&World::type);

				WorldForEachScale(world, [&](ScalePtr scale)
				{
					unordered_erase(simulation.spatial_scales, scale);

					unordered_erase(type.scales, scale);
				});

				unordered_erase(type.worlds, world);

				unordered_erase_it(simulation.spatial_worlds, it);

				DestroyWorld(type, world);
			}
			else
			{
				it++;
			}
		}
	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, WorldPtr world)
	{
		WorldDoNodeUnloadCommands(world);

		if (!simulation.unloading)
		{
			WorldDoNodeLoadCommands(world, simulation.frame_start_time);
		}
	}

	void ScaleUpdate(Simulation& simulation, ScalePtr scale)
	{
		ScaleUnloadUnutilizedNodes(scale, simulation.frame_start_time);

		if (!simulation.unloading)
		{
			ScaleLoadNodesAroundLoaders(scale, simulation.frame_start_time);

			ScaleUpdateEntityNodes(scale);
		}
	}

	WorldPtr GetEntityWorld(entity::WRef entity)
	{
		return entity->*&CWorld::world;
	}

	WorldPtr EntitySetWorld(Simulation& simulation, entity::WRef entity, WorldPtr world)
	{
		WorldPtr old_world = entity->*&CWorld::world;

		entity->*&CWorld::world = world;

		return old_world;
	}
}