#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "GalaxyComponents.h"
#include "Universe.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseNodeProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos)
		{
			UniverseNode* node = new UniverseNode();

			for (size_t i = 0; i < 8; i++)
			{
				flecs::entity galaxy = world.entity()
					.child_of(universe_entity)
					.add<GalaxyComponent>()
					.add<UniverseObjectComponent>()
					.add<SpatialEntity3DComponent>();

				node->entities.push_back(galaxy);
			}

			return node;
		}

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node)
		{
			UniverseNode* universe_node = static_cast<UniverseNode*>(node);

			for (flecs::entity_t entity : universe_node->entities)
			{
				if (world.is_alive(entity))
				{
					flecs::entity(world, entity).destruct();
				}
			}

			delete universe_node;
		}

		flecs::world& world;
		flecs::entity_t universe_entity;
		UniverseComponent& universe;

	};

	void UniverseSpatialCommands(flecs::entity entity, UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, UniverseNodeProcessor{ entity.world(), entity, universe });
	}

	struct GalaxyNodeProcessor
	{
		SpatialNode3D* CreateNode(size_t scale, godot::Vector3i pos)
		{
			return new SpatialNode3D();
		}

		void DestroyNode(size_t scale, godot::Vector3i pos, SpatialNode3D* node)
		{
			delete node;
		}
	};

	void GalaxySpatialCommands(flecs::entity entity, SimulatedGalaxyComponent& simulated_galaxy, SpatialWorld3DComponent& spatial_world)
	{
		SpatialCommands3DProcessor(entity, spatial_world, GalaxyNodeProcessor());
	}

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();
		world.import<SpatialComponents>();

		world.system<UniverseComponent, SpatialWorld3DComponent>("UniverseSpatialCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(UniverseSpatialCommands);

		world.system<SimulatedGalaxyComponent, SpatialWorld3DComponent>("GalaxySpatialCommands")
			.multi_threaded()
			.kind<WorldProgressPhase>()
			.each(GalaxySpatialCommands);
	}
}