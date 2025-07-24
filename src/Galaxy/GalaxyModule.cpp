#include "GalaxyModule.h"

#include "Components.h"
#include "UniverseSimulation.h"

#include "Entity/EntityModule.h"
#include "Spatial3D/SpatialModule.h"
#include "Render/RenderModule.h"
#include "DebugRender/DebugRenderModule.h"
#include "Universe/UniverseModule.h"

#include "GalaxyWorld.h"
#include "Universe/UniverseWorld.h"

#include "Util/Debug.h"

namespace voxel_game::galaxy
{
	const entity::TypeID k_galaxy_type = entity::Factory::Archetype::CreateTypeID<
#if defined(DEBUG_ENABLED)
		CName,
#endif
		CGalaxy,
		CRelationship,
		CPosition,
		CRotation,
		CEntity,
		CWorld,
		CLoader,
		CTransform
	>();

	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, UUID id, spatial3d::WorldPtr universe_world)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);
		DEBUG_ASSERT(!simulation.unloading, "We shouldn't create an entity while unloading");

		// Create the simulated galaxy
		entity::Ref galaxy_entity = SimulationCreateEntity(simulation, GenerateUUID(), k_galaxy_type);

		godot::String path = simulation.path.path_join("Galaxies").path_join(id.ToGodotString());

#if defined(DEBUG_ENABLED)
		galaxy_entity->*&CName::name = "SimulatedGalaxy";
#endif
		galaxy_entity->*&CWorld::path = path;
		galaxy_entity->*&CWorld::type = WorldConstructType::Galaxy;

		galaxy_entity->*&CEntity::parent_world = universe_world;

		// We want the simulated galaxy to load all galaxies around it
		galaxy_entity->*&CLoader::dist_per_lod = 3;
		galaxy_entity->*&CLoader::min_lod = 0;
		galaxy_entity->*&CLoader::max_lod = spatial3d::k_max_world_scale;

		return galaxy_entity;
	}

	void OnUpdateGalaxyEntity(Simulation& simulation, entity::WRef entity)
	{

	}

	void OnLoadGalaxyEntity(Simulation& simulation, entity::WRef entity)
	{
		simulation.galaxies.push_back(entity::Ref(entity));
	}

	void OnUnloadGalaxyEntity(Simulation& simulation, entity::WRef entity)
	{
		unordered_erase(simulation.galaxies, entity::Ref(entity));
	}

	void Initialize(Simulation& simulation)
	{
		simulation.galaxy_type.node_type.AddType<spatial3d::Node>();
		simulation.galaxy_type.node_type.AddType<spatial3d::PartialNode>();
		simulation.galaxy_type.node_type.AddType<spatial3d::LocalNode>();
		simulation.galaxy_type.node_type.AddType<spatial3d::RemoteNode>();
		simulation.galaxy_type.node_type.AddType<Node>();

		simulation.galaxy_type.scale_type.AddType<spatial3d::Scale>();
		simulation.galaxy_type.scale_type.AddType<spatial3d::PartialScale>();
		simulation.galaxy_type.scale_type.AddType<Scale>();

		simulation.galaxy_type.world_type.AddType<spatial3d::World>();
		simulation.galaxy_type.world_type.AddType<spatial3d::PartialWorld>();
		simulation.galaxy_type.world_type.AddType<spatial3d::LocalWorld>();
		simulation.galaxy_type.world_type.AddType<spatial3d::RemoteWorld>();
		simulation.galaxy_type.world_type.AddType<World>();

		simulation.galaxy_type.world_updates.push_back(&spatial3d::WorldUpdate);
		simulation.galaxy_type.world_updates.push_back(&debugrender::WorldUpdate);
		simulation.galaxy_type.world_updates.push_back(&universe::WorldUpdate);

		simulation.galaxy_type.scale_updates.push_back(&spatial3d::ScaleUpdate);
		simulation.galaxy_type.scale_updates.push_back(&debugrender::ScaleUpdate);
		simulation.galaxy_type.scale_updates.push_back(&universe::ScaleUpdate);

		simulation.entity_factory.AddCallback<CGalaxy>(PolyEvent::MainUpdate, cb::BindArg<&OnUpdateGalaxyEntity>(simulation));
		simulation.entity_factory.AddCallback<CGalaxy>(PolyEvent::BeginLoad, cb::BindArg<&OnLoadGalaxyEntity>(simulation));
		simulation.entity_factory.AddCallback<CGalaxy>(PolyEvent::BeginUnload, cb::BindArg<&OnUnloadGalaxyEntity>(simulation));
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