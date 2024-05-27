#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Util/VectorHelpers.h"
#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseLoadNodeCommandProcessor
	{
		flecs::entity world_entity;

		UniverseLoadNodeCommandProcessor(flecs::entity entity, SpatialWorld3DComponent& spatial_world)
		{
			world_entity = entity;
		}

		void Process(SpatialScale3D& spatial_scale, SpatialNode3D& spatial_node)
		{
			flecs::world world = world_entity.world();

			UniverseNode& universe_node = static_cast<UniverseNode&>(spatial_node);

			for (size_t i = 0; i < 0; i++)
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

		world.observer<SpatialWorld3DComponent, UniverseComponent>("UniverseInitializeSpatialProcessors")
			.event(flecs::OnAdd)
			.each([](SpatialWorld3DComponent& spatial_world, UniverseComponent& universe)
		{
			DEBUG_ASSERT(spatial_world.node_builder.node_create == nullptr, "The node builder was already initialized");

			spatial_world.node_builder = SpatialNodeBuilder<UniverseNode>();

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor>());
		});
	}
}