#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyRenderModule.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Physics/PhysicsComponents.h"

#include "Util/VectorHelpers.h"
#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseLoadNodeCommandProcessor
	{
		flecs::entity world_entity;

		UniverseLoadNodeCommandProcessor(flecs::entity entity)
		{
			world_entity = entity;
		}

		void Process(SpatialWorld3DComponent& spatial_world, SpatialScale3D& spatial_scale, SpatialNode3D& spatial_node)
		{
			flecs::scoped_world world = world_entity.scope();

			UniverseNode& universe_node = static_cast<UniverseNode&>(spatial_node);

			uint32_t s = 16 << spatial_node.coord.scale;

			for (size_t i = 0; i < 16; i++)
			{
				godot::Vector3 position = spatial_node.coord.pos * s;
				position += godot::Vector3(godot::UtilityFunctions::randf_range(0, s), godot::UtilityFunctions::randf_range(0, s), godot::UtilityFunctions::randf_range(0, s));

				flecs::entity galaxy = world.entity()
					.add<GalaxyComponent>()
					.set<Position3DComponent>({ position });

				universe_node.entities.push_back(galaxy);
			}
		}
	};

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseInitializeSpatialProcessors"))
			.event(flecs::OnAdd)
			.yield_existing()
			.term_at(2).filter()
			.each([](const UniverseComponent, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(spatial_world.node_builder.node_create == nullptr, "The node builder was already initialized");

			spatial_world.node_builder = SpatialNodeBuilder<UniverseNode>();

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor>());
		});
	}
}