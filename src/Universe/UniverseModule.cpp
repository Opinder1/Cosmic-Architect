#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseLoadNodeCommandProcessor
	{
		flecs::world& world;
		flecs::entity_t world_entity;
		const UniverseComponent* universe;

		UniverseLoadNodeCommandProcessor(flecs::entity entity, SpatialWorld3DComponent& spatial_world) :
			world(entity.world())
		{
			world_entity = entity;
			universe = entity.get<UniverseComponent>();
		}

		void Process(SpatialScale3D& spatial_scale, SpatialNode3D& spatial_node)
		{
			UniverseNode& universe_node = static_cast<UniverseNode&>(spatial_node);

			for (size_t i = 0; i < 10; i++)
			{
				flecs::entity galaxy = world.entity()
					.add<GalaxyComponent>()
					.child_of(world_entity);

				universe_node.entities.push_back(galaxy);
			}
		}
	};

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.observer<UniverseComponent, SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each([](UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.node_builder = SpatialNodeBuilder<UniverseNode>();

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor>());
		});
	}
}