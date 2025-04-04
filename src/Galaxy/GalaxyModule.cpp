#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "UniverseSimulation.h"

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
	void Initialize(Simulation& simulation)
	{
		simulation.galaxy_types.node_type.AddType<spatial3d::Node>();
		simulation.galaxy_types.node_type.AddType<Node>();

		simulation.galaxy_types.scale_type.AddType<spatial3d::Scale>();
		simulation.galaxy_types.scale_type.AddType<Scale>();

		simulation.galaxy_types.world_type.AddType<spatial3d::World>();
	}

	void Uninitialize(Simulation& simulation)
	{

	}

	void Update(Simulation& simulation)
	{

	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	entity::Ref CreateGalaxy(Simulation& simulation, spatial3d::NodeRef node, godot::Vector3 position, godot::Vector3 scale)
	{
		entity::Ref galaxy = simulation.entity_factory.GetPoly(GenerateUUID());

		simulation.entity_factory.AddTypes<
			galaxy::CWorld,
			physics3d::CPosition,
			physics3d::CScale,
			physics3d::CPosition,
			physics3d::CScale,
			spatial3d::CEntity
		>(galaxy.GetID());

		galaxy->*&physics3d::CPosition::position = position;
		galaxy->*&physics3d::CScale::scale = scale;

		(node->*&spatial3d::Node::entities).insert(galaxy.Reference());
		(node->*&universe::Node::galaxies).push_back(galaxy.Reference());

		return galaxy;
	}

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, const godot::String& path, entity::WRef universe_entity)
	{
		// Create the simulated galaxy
		entity::Ref galaxy_entity = simulation.entity_factory.GetPoly(GenerateUUID());

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
		galaxy_entity->*&spatial3d::CLoader::dist_per_lod = 3;
		galaxy_entity->*&spatial3d::CLoader::min_lod = 0;
		galaxy_entity->*&spatial3d::CLoader::max_lod = spatial3d::k_max_world_scale;

		//flecs::entity entity_loader = world.entity();

		//entity_loader.emplace<loading::CEntityDatabase>(path.path_join("entities.db"));
		//entity_loader.child_of(galaxy_entity);

		return galaxy_entity;
	}

	void DestroySimulatedGalaxy(Simulation& simulation, entity::WRef galaxy)
	{

	}
}