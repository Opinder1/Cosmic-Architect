#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialCommands.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyModule::GalaxyModule(flecs::world& world)
	{
		world.module<GalaxyModule>("GalaxyModule");

		world.import<SpatialComponents>();
		world.import<GalaxyComponents>();

		world.observer<GalaxyComponent, SpatialWorld3DComponent>()
			.event(flecs::OnAdd)
			.each([](GalaxyComponent& universe, SpatialWorld3DComponent& spatial_world)
		{
			spatial_world.max_scale = 16;

			for (size_t i = 0; i < spatial_world.max_scale; i++)
			{
				spatial_world.scales[i] = std::make_unique<GalaxyScale>();
			}

			spatial_world.node_builder = SpatialNodeBuilder<GalaxyNode>();
		});
	}
}