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
			spatial_world.node_builder = SpatialNodeBuilder<GalaxyNode>();
		});
	}
}