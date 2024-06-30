#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Galaxy/GalaxyComponents.h"
#include "Galaxy/GalaxyRenderModule.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	struct UniverseLoadNodeCommandProcessor
	{
		UniverseLoadNodeCommandProcessor(flecs::entity entity)
		{

		}

		void Process(SpatialWorld3DComponent& spatial_world, UniverseScale& universe_scale, UniverseNode& universe_node)
		{
			PARALLEL_ACCESS(spatial_world);

			uint32_t s = 16 << universe_node.coord.scale;

			for (size_t i = 0; i < 16; i++)
			{
				godot::Vector3 position = universe_node.coord.pos * s;
				position += godot::Vector3(godot::UtilityFunctions::randf_range(0, s), godot::UtilityFunctions::randf_range(0, s), godot::UtilityFunctions::randf_range(0, s));

				universe_node.galaxies_to_load.push_back(position);
			}
		}
	};

	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<PhysicsComponents>();
		world.import<SpatialComponents>();
		world.import<UniverseComponents>();

		// Initialise the spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.term_at(2).filter()
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			DEBUG_ASSERT(spatial_world.max_scale == 0, "The spatial world was already initialized with a type");

			spatial_world.max_scale = 16;

			spatial_world.builder = SpatialBuilder<UniverseScale, UniverseNode>();

			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}

			spatial_world.load_command_processors.push_back(SpatialNodeCommandProcessor<UniverseLoadNodeCommandProcessor, UniverseScale, UniverseNode>());
		});

		// Uninitialize spatial world of a universe
		world.observer<const UniverseComponent, SpatialWorld3DComponent>(DEBUG_ONLY("UniverseUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.yield_existing()
			.term_at(2).filter()
			.each([](const UniverseComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}