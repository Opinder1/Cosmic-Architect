#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Universe/UniverseSimulation.h"

#include "Spatial3D/SpatialModule.h"
#include "Render/RenderModule.h"

#include "Universe/UniverseComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Simulation/SimulationComponents.h"
#include "Loading/LoadingComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Render/RenderComponents.h"
#include "Entity/EntityComponents.h"

#include "Util/Debug.h"

namespace voxel_game::galaxy
{
	void Initialize(universe::Simulation& simulation)
	{
		simulation.galaxy_types.node_type.AddType<spatial3d::Node>();
		simulation.galaxy_types.node_type.AddType<Node>();

		simulation.galaxy_types.scale_type.AddType<spatial3d::Scale>();
		simulation.galaxy_types.scale_type.AddType<Scale>();

		simulation.galaxy_types.world_type.AddType<spatial3d::World>();
	}

	void Uninitialize(universe::Simulation& simulation)
	{

	}

	void Update(universe::Simulation& simulation)
	{

	}

	void WorkerUpdate(universe::Simulation& simulation, size_t index)
	{

	}

	entity::Ptr CreateNewSimulatedGalaxy(universe::Simulation& simulation, const godot::String& path, entity::Ptr universe_entity)
	{
		// Create the simulated galaxy
		entity::Ptr galaxy_entity = simulation.entity_factory.CreatePoly(GenerateUUID());

#if defined(DEBUG_ENABLED)
		simulation.entity_factory.AddTypes<entity::CName>(galaxy_entity.GetID());
		galaxy_entity->*&entity::CName::name = "SimulatedGalaxy";
#endif

		simulation.entity_factory.AddTypes<
			entity::CParent,
			physics3d::CPosition,
			physics3d::CRotation,
			CWorld,
			spatial3d::CWorld,
			spatial3d::CLoader,
			simulation::CPath
		>(galaxy_entity.GetID());

		if (rendering::IsEnabled())
		{
			simulation.entity_factory.AddTypes<rendering::CTransform>(galaxy_entity.GetID());
		}

		galaxy_entity->*&entity::CParent::parent = universe_entity;

		galaxy_entity->*&spatial3d::CWorld::world = spatial3d::CreateWorld(simulation.galaxy_types, spatial3d::k_max_world_scale);

		// We want the simulated galaxy to load all galaxies around it
		(galaxy_entity->*&spatial3d::CLoader::loader)->dist_per_lod = 3;
		(galaxy_entity->*&spatial3d::CLoader::loader)->min_lod = 0;
		(galaxy_entity->*&spatial3d::CLoader::loader)->max_lod = spatial3d::k_max_world_scale;

		//flecs::entity entity_loader = world.entity();

		//entity_loader.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));
		//entity_loader.child_of(galaxy_entity);

		return galaxy_entity;
	}

	void DestroySimulatedGalaxy(universe::Simulation& simulation, entity::Ptr galaxy)
	{
		simulation.entity_factory.DestroyPoly(galaxy.GetID());
	}
}