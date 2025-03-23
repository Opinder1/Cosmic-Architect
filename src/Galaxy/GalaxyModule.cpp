#include "GalaxyModule.h"
#include "GalaxyComponents.h"
#include "GalaxyPrefabs.h"

#include "Universe/UniverseComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Loading/LoadingComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Render/RenderModule.h"
#include "Render/RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	spatial3d::Types galaxy_world_types;

	flecs::entity CreateNewSimulatedGalaxy(flecs::world& world, const godot::String& path, flecs::entity_t universe_entity)
	{
		// Create the simulated galaxy
		flecs::entity galaxy_entity = world.entity();

#if defined(DEBUG_ENABLED)
		galaxy_entity.set_name("SimulatedGalaxy");
#endif

		galaxy_entity.is_a<PSimulatedGalaxy>();
		galaxy_entity.child_of(universe_entity);
		galaxy_entity.emplace<sim::CPath>(path);

		if (rendering::IsEnabled())
		{
			galaxy_entity.add<rendering::CTransform>();
		}

		spatial3d::CWorld* spatial_world = galaxy_entity.get_mut<spatial3d::CWorld>();

		spatial_world->world = spatial3d::CreateWorld(galaxy_world_types, spatial3d::k_max_world_scale);

		spatial3d::InitializeWorldScaleEntities(galaxy_entity, spatial_world->world);

		// We want the simulated galaxy to load all galaxies around it
		spatial3d::CLoader* spatial_loader = galaxy_entity.get_mut<spatial3d::CLoader>();
		spatial_loader->loader->dist_per_lod = 3;
		spatial_loader->loader->min_lod = 0;
		spatial_loader->loader->max_lod = spatial3d::k_max_world_scale;

		flecs::entity entity_loader = world.entity();

		entity_loader.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));
		entity_loader.child_of(galaxy_entity);

		return galaxy_entity;
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<Prefabs>();
		world.import<spatial3d::Components>();
		world.import<physics3d::Components>();
		world.import<loading::Components>();
		world.import<rendering::Components>();
		world.import<sim::Components>();

		galaxy_world_types.node_type.AddType<spatial3d::Node>();
		galaxy_world_types.scale_type.AddType<spatial3d::Scale>();
		galaxy_world_types.world_type.AddType<spatial3d::World>();
		galaxy_world_types.node_type.AddType<Node>();
		galaxy_world_types.scale_type.AddType<Scale>();

		// Initialise the spatial world of a galaxy
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{

		});
	}
}