#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyPrefabs.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Voxel/VoxelComponents.h" // Testing
#include "VoxelRender/VoxelRenderComponents.h" // Testing

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"
#include "Simulation/SimulationModule.h"

#include "Render/RenderComponents.h"
#include "Render/RenderModule.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::universe
{
	flecs::entity CreateNewUniverse(flecs::world& world, const godot::StringName& path)
	{
		// Create the universe
		flecs::entity universe_entity = world.entity();

#if defined(DEBUG_ENABLED)
		universe_entity.set_name("Universe");
#endif

		universe_entity.emplace<sim::CPath>(path);
		universe_entity.add<universe::CWorld>();
		universe_entity.add<voxel::CWorld>(); // Testing
		if (rendering::IsEnabled())
		{
			universe_entity.add<voxelrender::CWorld>(); // Testing
		}

		spatial3d::CWorld& spatial_world = universe_entity.ensure<spatial3d::CWorld>();

		DEBUG_ASSERT(!world.is_deferred(), "Observers need to be invoked to initialize poly types");

		spatial_world.world = spatial3d::CreateWorld(spatial_world.types, spatial3d::k_max_world_scale);
		spatial_world.world->node_size = 16;
		spatial_world.world->node_keepalive = 1s;

		spatial3d::InitializeWorldScaleEntities(universe_entity, *spatial_world.world);

		if (rendering::IsEnabled())
		{
			universe_entity.add<rendering::CTransform>();

			spatial3d::Types& types = spatial_world.types;

			WORLD_TO(spatial_world.world, voxelrender::World)->voxel_material = rendering::AllocRID(rendering::RIDType::Material);
		}

		return universe_entity;
	}

	// Spawns a bunch of random cubes around the camera
	struct UniverseNodeLoaderTest1
	{
		flecs::entity entity;
		const spatial3d::Types& types;
		const spatial3d::World& world;

		void LoadNode(spatial3d::Node* node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node->scale_index;
			const uint32_t scale_node_step = scale_step * world.node_size;
			const double box_size = double(scale_step) / 2.0;

			for (size_t i = 0; i < entities_per_node; i++)
			{
				double position_x = node->position.x * scale_node_step;
				double position_y = node->position.y * scale_node_step;
				double position_z = node->position.z * scale_node_step;

				position_x += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_y += godot::UtilityFunctions::randf_range(0, scale_node_step);
				position_z += godot::UtilityFunctions::randf_range(0, scale_node_step);

				flecs::entity galaxy = sim::GetPool().CreateEntity();

				galaxy.child_of(entity);
				galaxy.is_a<galaxy::GalaxyPrefab>();
				galaxy.set(physics3d::CPosition{ godot::Vector3(position_x, position_y, position_z) });
				galaxy.set(physics3d::CScale{ godot::Vector3(box_size, box_size, box_size) });
				galaxy.add<rendering::CTransform>();
				spatial3d::CEntity& entity = galaxy.ensure<spatial3d::CEntity>();

				node->entities.insert(entity.entity);
				NODE_TO(node, Node)->galaxies.push_back(galaxy);
			}
		}
	};

	// Spawns a bunch of flat squares around the camera on the xz plane
	struct UniverseNodeLoaderTest2
	{
		flecs::entity entity;
		const spatial3d::Types& types;
		const spatial3d::World& world;

		void LoadNode(spatial3d::Node& node)
		{
			if (node.position.y != 0)
			{
				return;
			}

			const uint32_t scale_step = 1 << node.scale_index;
			const int32_t scale_node_step = scale_step * world.node_size;
			const uint8_t box_shrink = 2;

			int32_t position_x = node.position.x * scale_node_step;
			int32_t position_y = node.position.y * scale_node_step;
			int32_t position_z = node.position.z * scale_node_step;

			flecs::entity galaxy = sim::GetPool().CreateEntity();

			galaxy.child_of(entity);
			galaxy.is_a<galaxy::GalaxyPrefab>();
			galaxy.emplace<physics3d::CPosition>(godot::Vector3i{ position_x, position_y - node.scale_index - 1, position_z });
			galaxy.emplace<physics3d::CScale>(godot::Vector3i{scale_node_step / 4, 1, scale_node_step / 4});
			galaxy.add<rendering::CTransform>();
			spatial3d::CEntity& entity = galaxy.ensure<spatial3d::CEntity>();

			node.entities.insert(entity.entity);
			NODE_TO(node, Node).galaxies.push_back(galaxy);
		}

		void UnloadNode(spatial3d::Node& node)
		{
			const uint32_t entities_per_node = 4;
			const uint32_t scale_step = 1 << node.scale_index;
			const uint32_t scale_node_step = scale_step * world.node_size;

			for (flecs::entity_t galaxy : NODE_TO(node, Node).galaxies)
			{
				flecs::entity(entity.world(), galaxy).destruct();
			}
		}
	};

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<physics3d::Components>();
		world.import<spatial3d::Components>();
		world.import<voxel::Components>(); // Testing
		world.import<voxelrender::Components>(); // Testing
		world.import<universe::Components>();
		world.import<galaxy::Prefabs>();

		spatial3d::NodeType::RegisterType<Node>();
		spatial3d::ScaleType::RegisterType<Scale>();
		spatial3d::WorldType::RegisterType<World>();

		// Initialise the spatial world of a universe
		world.observer<spatial3d::CWorld>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.with<const CWorld>()
			.each([](spatial3d::CWorld& spatial_world)
		{
			spatial_world.types.node_type.AddType<Node>();
			spatial_world.types.scale_type.AddType<Scale>();
			spatial_world.types.world_type.AddType<World>();
		});

		world.system<spatial3d::CScale, const spatial3d::CWorld>(DEBUG_ONLY("UniverseLoadSpatialNode"))
			.multi_threaded()
			.term_at(1).up(flecs::ChildOf)
			.with<const CWorld>().up(flecs::ChildOf)
			.each([](flecs::entity entity, spatial3d::CScale& spatial_scale, const spatial3d::CWorld& spatial_world)
		{
			UniverseNodeLoaderTest2 loader{ entity, spatial_world.types, *spatial_world.world };

			for (spatial3d::Node* node : spatial_scale.scale->load_commands)
			{
				loader.LoadNode(*node);
			}

			for (spatial3d::Node* node : spatial_scale.scale->unload_commands)
			{
				loader.UnloadNode(*node);
			}
		});
	}
}