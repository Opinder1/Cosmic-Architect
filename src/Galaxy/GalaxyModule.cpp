#include "GalaxyModule.h"
#include "GalaxyComponents.h"

#include "Galaxy/GalaxyComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GalaxyModule::GalaxyModule(flecs::world& world)
	{
		world.module<GalaxyModule>("GalaxyModule");

		world.import<SpatialComponents>();
		world.import<GalaxyComponents>();
	}
}