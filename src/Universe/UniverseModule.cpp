#include "UniverseModule.h"
#include "UniverseSimulation.h"

#include "Render/RenderModule.h"
#include "Galaxy/GalaxyModule.h"
#include "Entity/EntityModule.h"
#include "Spatial3D/SpatialModule.h"

#include "UniverseComponents.h"
#include "Entity/EntityComponents.h"
#include "Galaxy/GalaxyComponents.h"
#include "Spatial3D/SpatialComponents.h"
#include "Physics3D/PhysicsComponents.h"
#include "Render/RenderComponents.h"

#include "UniverseWorld.h"

#include "Util/Debug.h"

namespace voxel_game::universe
{
	void LoadNodeRandomly(Simulation& simulation, spatial3d::WorldPtr world, spatial3d::NodePtr node)
	{
		const uint32_t entities_per_node = 4;
		const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
		const uint32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;
		const double box_size = double(scale_step) / 2.0;

		for (size_t i = 0; i < entities_per_node; i++)
		{
			godot::Vector3 position = node->*&spatial3d::Node::position * scale_node_step;

			position.x += godot::UtilityFunctions::randf_range(0, scale_node_step);
			position.y += godot::UtilityFunctions::randf_range(0, scale_node_step);
			position.z += godot::UtilityFunctions::randf_range(0, scale_node_step);

			//galaxy::CreateGalaxy(simulation, node, position, godot::Vector3(box_size, box_size, box_size), world);
		}
	}

	entity::Ref CreateUniverse(Simulation& simulation, UUID id)
	{
		DEBUG_THREAD_CHECK_WRITE(&simulation);

		entity::Ref universe_entity = simulation.entity_factory.GetPoly(id);

		// Create the universe
		simulation.entity_factory.AddTypes<
			universe::CUniverse,
			entity::CRelationship,
			spatial3d::CWorld,
			rendering::CScenario
		>(universe_entity.GetID());

		if (rendering::IsEnabled())
		{
			simulation.entity_factory.AddTypes<rendering::CTransform>(universe_entity.GetID());
		}

		spatial3d::WorldPtr world = spatial3d::CreateWorld(simulation.universe_type, simulation.path);

		world->*&spatial3d::World::node_size = 16;
		world->*&spatial3d::PartialWorld::node_keepalive = 1s;

		spatial3d::EntitySetWorld(simulation, universe_entity, world);

		entity::OnLoadEntity(simulation, universe_entity);

		return universe_entity;
	}

	void OnUpdateUniverseEntity(Simulation& simulation, entity::WRef entity)
	{

	}

	void OnLoadUniverseEntity(Simulation& simulation, entity::WRef entity)
	{
		simulation.universes.push_back(entity::Ref(entity));
	}

	void OnUnloadUniverseEntity(Simulation& simulation, entity::WRef entity)
	{
		unordered_erase(simulation.universes, entity::Ref(entity));
	}

	void SerializeUniverseNode(Simulation& simulation, spatial3d::WorldPtr world, spatial3d::NodePtr node, serialize::Writer& writer)
	{
		size_t version = 0;
		writer.Write(version);

		writer.Write(node->*&spatial3d::Node::scale_index);

		writer.Write((node->*&Node::galaxies).size());

		for (entity::WRef galaxy : node->*&Node::galaxies)
		{
			writer.Write(galaxy.GetID());
		}
	}

	void DeserializeUniverseNode(Simulation& simulation, spatial3d::WorldPtr world, spatial3d::NodePtr node, serialize::Reader& reader)
	{
		size_t version = 0;
		reader.Read(version);

		reader.Read(node->*&spatial3d::Node::scale_index);

		size_t galaxy_count;
		reader.Read(galaxy_count);

		for (size_t i = 0; i < galaxy_count; i++)
		{
			UUID id;
			reader.Read(id);

			entity::TypeID types;
			reader.Read(types);

			entity::Ref entity = simulation.entity_factory.GetPoly(id);

			simulation.entity_factory.AddTypes(id, types);

			(node->*&spatial3d::Node::entities).insert(entity.Reference());
			(node->*&Node::galaxies).push_back(entity.Reference());
		}
	}

	const entity::TypeID galaxy_type = entity::Factory::Archetype::CreateTypeID<
		galaxy::CGalaxy,
		entity::CRelationship,
		physics3d::CPosition,
		physics3d::CScale,
		physics3d::CRotation,
		spatial3d::CEntity>();

	void GenerateUniverseNode(Simulation& simulation, spatial3d::WorldPtr world, spatial3d::NodePtr node)
	{
		if ((node->*&spatial3d::Node::position).y != 0)
		{
			return;
		}

		const uint32_t scale_step = 1 << node->*&spatial3d::Node::scale_index;
		const int32_t scale_node_step = scale_step * world->*&spatial3d::World::node_size;

		godot::Vector3i position = node->*&spatial3d::Node::position * scale_node_step;

		position.y -= node->*&spatial3d::Node::scale_index - 1;

		entity::Ref galaxy_entity = simulation.entity_factory.GetPoly(GenerateUUID());

		simulation.entity_factory.AddTypes(galaxy_entity.GetID(), galaxy_type);
		
		galaxy_entity->*&physics3d::CPosition::position = position;
		galaxy_entity->*&physics3d::CScale::scale = godot::Vector3i{ scale_node_step / 4, 1, scale_node_step / 4 };

		(node->*&spatial3d::Node::entities).insert(galaxy_entity.Reference());
		(node->*&Node::galaxies).push_back(galaxy_entity.Reference());
	}

	void Initialize(Simulation& simulation)
	{
		simulation.universe_type.node_type.AddType<spatial3d::Node>();
		simulation.universe_type.node_type.AddType<spatial3d::PartialNode>();
		simulation.universe_type.node_type.AddType<spatial3d::LocalNode>();
		simulation.universe_type.node_type.AddType<Node>();

		simulation.universe_type.scale_type.AddType<spatial3d::Scale>();
		simulation.universe_type.scale_type.AddType<spatial3d::PartialScale>();
		simulation.universe_type.scale_type.AddType<Scale>();

		simulation.universe_type.world_type.AddType<spatial3d::World>();
		simulation.universe_type.world_type.AddType<spatial3d::PartialWorld>();
		simulation.universe_type.world_type.AddType<spatial3d::LocalWorld>();
		simulation.universe_type.world_type.AddType<World>();

		simulation.universe_type.serialize_callbacks.push_back(cb::BindArg<&SerializeUniverseNode>(simulation));
		simulation.universe_type.deserialize_callbacks.push_back(cb::BindArg<&DeserializeUniverseNode>(simulation));
		simulation.universe_type.generate_callbacks.push_back(cb::BindArg<&GenerateUniverseNode>(simulation));

		simulation.entity_factory.AddCallback<CUniverse>(PolyEvent::MainUpdate, cb::BindArg<&OnUpdateUniverseEntity>(simulation));
		simulation.entity_factory.AddCallback<CUniverse>(PolyEvent::BeginLoad, cb::BindArg<&OnLoadUniverseEntity>(simulation));
		simulation.entity_factory.AddCallback<CUniverse>(PolyEvent::BeginUnload, cb::BindArg<&OnUnloadUniverseEntity>(simulation));
	}

	void Uninitialize(Simulation& simulation)
	{
		simulation.universes.clear();
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