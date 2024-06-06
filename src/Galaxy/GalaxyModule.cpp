#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyModule::GalaxyModule(flecs::world& world)
	{
		world.module<GalaxyModule>("GalaxyModule");

		world.import<SpatialComponents>();
		world.import<GalaxyComponents>();

		world.observer<const GalaxyComponent, SpatialWorld3DComponent>(DEBUG_ONLY("GalaxyUninitializeSpatialWorld"))
			.event(flecs::OnAdd)
			.each([](const GalaxyComponent& galaxy, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.max_scale = 16;

			spatial_world.builder = SpatialBuilder<GalaxyScale, GalaxyNode>();

			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = spatial_world.builder.scale_create();
			}
		});

		world.observer<const GalaxyComponent, SpatialWorld3DComponent>(DEBUG_ONLY("GalaxyUninitializeSpatialWorld"))
			.event(flecs::OnRemove)
			.yield_existing()
			.term_at(2).filter()
			.each([](const GalaxyComponent& galaxy, SpatialWorld3DComponent& spatial_world)
		{
			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.builder.scale_destroy(spatial_world.scales[i]);
			}
		});
	}
}