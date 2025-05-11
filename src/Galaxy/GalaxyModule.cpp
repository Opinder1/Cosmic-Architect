#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "UniverseSimulation.h"

#include "Spatial3D/SpatialModule.h"
#include "Render/RenderModule.h"

#include "Universe/UniverseComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Loading/LoadingComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Render/RenderComponents.h"
#include "Entity/EntityComponents.h"

#include "GalaxyWorld.h"
#include "Universe/UniverseWorld.h"
#include "Loading/LoadingWorld.h"

#include "Util/Debug.h"

namespace voxel_game::galaxy
{
	entity::Ref CreateGalaxy(Simulation& simulation, spatial3d::NodePtr node, godot::Vector3 position, godot::Vector3 scale, entity::WRef universe_entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);
		DEBUG_ASSERT(!simulation.unloading, "We shouldn't create an entity while unloading");

		entity::Ref galaxy_entity = simulation.entity_factory.GetPoly(GenerateUUID());

		simulation.entity_factory.AddTypes<
			CGalaxy,
			entity::CParent,
			physics3d::CPosition,
			physics3d::CScale,
			physics3d::CRotation,
			spatial3d::CEntity
		>(galaxy_entity.GetID());

		galaxy_entity->*&physics3d::CPosition::position = position;
		galaxy_entity->*&physics3d::CScale::scale = scale;

		(node->*&spatial3d::Node::entities).insert(galaxy_entity.Reference());
		(node->*&universe::Node::galaxies).push_back(galaxy_entity.Reference());

		return galaxy_entity;
	}

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, const godot::String& path, entity::WRef universe_entity)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);
		DEBUG_ASSERT(!simulation.unloading, "We shouldn't create an entity while unloading");

		// Create the simulated galaxy
		entity::Ref galaxy_entity = simulation.entity_factory.GetPoly(GenerateUUID());

		simulation.entity_factory.AddTypes<
			CGalaxy,
			entity::CParent,
			physics3d::CPosition,
			physics3d::CRotation,
			spatial3d::CEntity,
			spatial3d::CWorld,
			spatial3d::CLoader
		>(galaxy_entity.GetID());

		if (rendering::IsEnabled())
		{
			simulation.entity_factory.AddTypes<rendering::CTransform>(galaxy_entity.GetID());
		}

#if defined(DEBUG_ENABLED)
		simulation.entity_factory.AddTypes<entity::CName>(galaxy_entity.GetID());
		galaxy_entity->*&entity::CName::name = "SimulatedGalaxy";
#endif

		galaxy_entity->*&entity::CParent::parent = entity::Ref(universe_entity);

		galaxy_entity->*&CGalaxy::path = path;

		spatial3d::WorldPtr world = spatial3d::CreateWorld(simulation.galaxy_type);

		loading::WorldOpenDatabase(simulation, world, path.path_join("stars.db"));

		galaxy_entity->*&spatial3d::CWorld::world = world;

		// We want the simulated galaxy to load all galaxies around it
		galaxy_entity->*&spatial3d::CLoader::dist_per_lod = 3;
		galaxy_entity->*&spatial3d::CLoader::min_lod = 0;
		galaxy_entity->*&spatial3d::CLoader::max_lod = spatial3d::k_max_world_scale;

		return galaxy_entity;
	}

	void OnLoadGalaxyEntity(Simulation& simulation, entity::EventData& data)
	{
		simulation.galaxies.push_back(entity::Ref(data.entity));
	}

	void OnUnloadGalaxyEntity(Simulation& simulation, entity::EventData& data)
	{
		unordered_erase(simulation.galaxies, entity::Ref(data.entity));
	}

	void Initialize(Simulation& simulation)
	{
		simulation.galaxy_type.node_type.AddType<spatial3d::Node>();
		simulation.galaxy_type.node_type.AddType<loading::Node>();
		simulation.galaxy_type.node_type.AddType<Node>();

		simulation.galaxy_type.scale_type.AddType<spatial3d::Scale>();
		simulation.galaxy_type.scale_type.AddType<spatial3d::PartialScale>();
		simulation.galaxy_type.scale_type.AddType<loading::Scale>();
		simulation.galaxy_type.scale_type.AddType<Scale>();

		simulation.galaxy_type.world_type.AddType<spatial3d::World>();
		simulation.galaxy_type.world_type.AddType<spatial3d::PartialWorld>();
		simulation.galaxy_type.world_type.AddType<loading::World>();
		simulation.galaxy_type.world_type.AddType<World>();

		simulation.entity_factory.AddCallback<CGalaxy>(entity::Event::Load, cb::Bind<&OnLoadGalaxyEntity>());
		simulation.entity_factory.AddCallback<CGalaxy>(entity::Event::Unload, cb::Bind<&OnUnloadGalaxyEntity>());
	}

	void Uninitialize(Simulation& simulation)
	{
		simulation.galaxies.clear();
	}

	bool IsUnloadDone(Simulation& simulation)
	{
		return true;
	}

	void Update(Simulation& simulation)
	{

	}

	void WorkerUpdate(Simulation& simulation, size_t index)
	{

	}

	void WorldUpdate(Simulation& simulation, spatial3d::WorldPtr world)
	{

	}

	void ScaleUpdate(Simulation& simulation, spatial3d::ScalePtr scale)
	{

	}
}