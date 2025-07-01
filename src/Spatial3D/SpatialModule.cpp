#include "SpatialModule.h"
#include "SpatialComponents.h"
#include "SpatialTraverse.h"

#include "UniverseSimulation.h"

#include "Entity/EntityComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	void OnLoadSpatialEntity(Simulation& simulation, entity::EventData& data)
	{
		simulation.spatial_worlds.push_back(data.entity->*&CWorld::world);

		WorldForEachScale(data.entity->*&CWorld::world, [&simulation](ScalePtr scale)
		{
			simulation.spatial_scales.push_back(scale);
		});
	}

	void OnUnloadSpatialEntity(Simulation& simulation, entity::EventData& data)
	{
		UnloadWorld(data.entity->*&CWorld::world);
	}

	void Initialize(Simulation& simulation)
	{
		simulation.entity_factory.AddCallback<CWorld>(entity::Event::BeginLoad, cb::Bind<OnLoadSpatialEntity>());
		simulation.entity_factory.AddCallback<CWorld>(entity::Event::BeginUnload, cb::Bind<OnUnloadSpatialEntity>());
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

			if (IsWorldUnloading(world) && GetNodeCount(world) == 0)
			{
				WorldForEachScale(world, [&simulation](ScalePtr scale)
				{
					unordered_erase(simulation.spatial_scales, scale);
				});

				unordered_erase_it(simulation.spatial_worlds, it);

				DestroyWorld(world);
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
		return entity->*&spatial3d::CWorld::world;
	}

	WorldPtr EntitySetWorld(Simulation& simulation, entity::WRef entity, WorldPtr world)
	{
		WorldPtr old_world = entity->*&spatial3d::CWorld::world;

		entity->*&spatial3d::CWorld::world = world;

		return old_world;
	}
}