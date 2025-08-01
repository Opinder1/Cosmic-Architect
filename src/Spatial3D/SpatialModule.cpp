#include "SpatialModule.h"
#include "SpatialTraverse.h"

#include "Components.h"
#include "UniverseSimulation.h"

#include "Util/Debug.h"
#include "Util/Callback.h"

#include <easy/profiler.h>

namespace voxel_game::spatial3d
{
	SpatialTypeData& GetType(Simulation& simulation, WorldConstructType type)
	{
		switch (type)
		{
		case WorldConstructType::Universe:
			return simulation.universe_type;

		case WorldConstructType::Galaxy:
			return simulation.galaxy_type;

		case WorldConstructType::StarSystem:
			return simulation.star_system_type;

		case WorldConstructType::Planet:
			return simulation.planet_type;

		case WorldConstructType::SpaceStation:
			return simulation.space_station_type;

		case WorldConstructType::SpaceShip:
			return simulation.space_ship_type;

		case WorldConstructType::Vehicle:
			return simulation.vehicle_type;

		default:
			DEBUG_CRASH("Invalid world construct type");
			return simulation.universe_type;
		}
	}

	void OnLoadSpatialEntity(Simulation& simulation, entity::WRef entity)
	{
		SpatialTypeData& type = GetType(simulation, entity->*&CWorld::type);

		godot::String path = type.path.path_join(entity.GetID().ToGodotString());

		spatial3d::WorldPtr world = spatial3d::CreateWorld(type, path);

		entity->*&CWorld::world = world;

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

		entity->*&CWorld::world = nullptr;
	}

	void OnLoadLoaderEntity(Simulation& simulation, entity::WRef entity)
	{
		std::vector<entity::WRef>& loaders = (entity->*&CEntity::parent_world)->*&PartialWorld::loaders;

		loaders.push_back(entity);
	}

	void OnUnloadLoaderEntity(Simulation& simulation, entity::WRef entity)
	{
		std::vector<entity::WRef>& loaders = (entity->*&CEntity::parent_world)->*&PartialWorld::loaders;

		unordered_erase(loaders, entity::Ref(entity));
	}

	void Initialize(Simulation& simulation)
	{
		simulation.entity_factory.AddCallback<CWorld>(PolyEvent::BeginLoad, cb::BindArg<OnLoadSpatialEntity>(simulation));
		simulation.entity_factory.AddCallback<CWorld>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadSpatialEntity>(simulation));
		simulation.entity_factory.AddCallback<CLoader, CEntity>(PolyEvent::BeginLoad, cb::BindArg<OnLoadLoaderEntity>(simulation));
		simulation.entity_factory.AddCallback<CLoader, CEntity>(PolyEvent::BeginUnload, cb::BindArg<OnUnloadLoaderEntity>(simulation));
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
}