#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "UniverseSimulation.h"

#include "Spatial3D/SpatialModule.h"
#include "Render/RenderModule.h"
#include "Entity/EntityModule.h"

#include "Universe/UniverseComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Render/RenderComponents.h"
#include "Entity/EntityComponents.h"

#include "GalaxyWorld.h"
#include "Universe/UniverseWorld.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/dir_access.hpp>

namespace voxel_game::galaxy
{
	entity::Ref CreateSimulatedGalaxy(Simulation& simulation, UUID id, spatial3d::WorldPtr universe_world)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);
		DEBUG_ASSERT(!simulation.unloading, "We shouldn't create an entity while unloading");

		// Create the simulated galaxy
		entity::Ref galaxy_entity = simulation.entity_factory.GetPoly(GenerateUUID());

		simulation.entity_factory.AddTypes<
			CGalaxy,
			entity::CRelationship,
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
		entity::SetDebugName(simulation, galaxy_entity, "SimulatedGalaxy");
#endif

		spatial3d::AddEntity(universe_world, galaxy_entity);

		godot::String path = simulation.path.path_join("Galaxies").path_join(id.ToGodotString());

		godot::DirAccess::make_dir_recursive_absolute(path);

		spatial3d::WorldPtr world = spatial3d::CreateWorld(simulation.galaxy_type, path);

		spatial3d::EntitySetWorld(simulation, galaxy_entity, world);

		// We want the simulated galaxy to load all galaxies around it
		galaxy_entity->*&spatial3d::CLoader::dist_per_lod = 3;
		galaxy_entity->*&spatial3d::CLoader::min_lod = 0;
		galaxy_entity->*&spatial3d::CLoader::max_lod = spatial3d::k_max_world_scale;

		entity::OnLoadEntity(simulation, galaxy_entity);

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