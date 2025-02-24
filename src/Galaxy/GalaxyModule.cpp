#include "GalaxyModule.h"
#include "GalaxyComponents.h"
#include "GalaxyPrefabs.h"

#include "Universe/UniverseComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Loading/LoadingComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Render/RenderComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxy
{
	flecs::entity CreateNewSimulatedGalaxy(flecs::world& world, const godot::String& path, flecs::entity_t universe_entity)
	{
		// Create the simulated galaxy
		flecs::entity galaxy_entity = world.entity();

#if defined(DEBUG_ENABLED)
		galaxy_entity.set_name("SimulatedGalaxy");
#endif

		galaxy_entity.emplace<sim::CPath>(path);

		loading::CDatabase& database = galaxy_entity.ensure<loading::CDatabase>();
		database.path = path.path_join("entities.db");

		galaxy_entity.child_of(universe_entity);

		galaxy_entity.add<CWorld>();

		spatial3d::CWorld& spatial_world = galaxy_entity.ensure<spatial3d::CWorld>();
		spatial_world.world->max_scale = spatial3d::k_max_world_scale;

		galaxy_entity.add<physics3d::CPosition>();
		galaxy_entity.add<physics3d::CRotation>();

		spatial3d::InitializeWorldScaleEntities(galaxy_entity, *spatial_world.world);

		// We want the simulated galaxy to load all galaxies around it
		spatial3d::Loader& spatial_loader = galaxy_entity.ensure<spatial3d::Loader>();
		spatial_loader.dist_per_lod = 3;
		spatial_loader.min_lod = 0;
		spatial_loader.max_lod = spatial3d::k_max_world_scale;

		galaxy_entity.add<rendering::CTransform>();

		return galaxy_entity;
	}

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<Prefabs>();
		world.import<spatial3d::Components>();
		world.import<physics3d::Components>();

		spatial3d::NodeType::RegisterType<Node>();
		spatial3d::ScaleType::RegisterType<Scale>();

		// Initialise the spatial world of a galaxy
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{
			spatial_world.types.node_type.AddType<Node>();
			spatial_world.types.scale_type.AddType<Scale>();
		});
	}
}