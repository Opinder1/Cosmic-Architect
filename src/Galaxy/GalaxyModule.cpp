#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Universe/UniverseComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Physics/PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyModule::GalaxyModule(flecs::world& world)
	{
		world.module<GalaxyModule>("GalaxyModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();
		world.import<SpatialComponents>();
		world.import<PhysicsComponents>();

		world.singleton<GalaxyComponent>()
			.add_second<UniverseComponent>(flecs::OneOf)
			.add_second<Position3DComponent>(flecs::With)
			.add_second<Rotation3DComponent>(flecs::With);

		world.singleton<GalaxyObjectComponent>()
			.add_second<GalaxyComponent>(flecs::OneOf)
			.add_second<SpatialEntity3DComponent>(flecs::With);

		world.singleton<StarComponent>()
			.add_second<GalaxyObjectComponent>(flecs::With);

		world.singleton<SimulatedGalaxyComponent>()
			.add_second<GalaxyComponent>(flecs::With)
			.add_second<SpatialWorld3DComponent>(flecs::With);

		// Initialise the spatial world of a galaxy
		world.observer<const GalaxyComponent, SpatialWorld3DComponent>(DEBUG_ONLY("GalaxyInitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.term_at(0).filter() // Only do the event when the spatial world is added
			.each([](const GalaxyComponent& galaxy, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.max_scale = k_max_world_scale;

			spatial_world.builder = SpatialBuilder<GalaxyScale, GalaxyNode>();

			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}
		});

		// Uninitialize spatial world of a galaxy
		world.observer<const GalaxyComponent, SpatialWorld3DComponent>(DEBUG_ONLY("GalaxyUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.term_at(0).filter() // Only do the event when the spatial world is added
			.each([](const GalaxyComponent& galaxy, SpatialWorld3DComponent& spatial_world)
		{
			for (uint8_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}