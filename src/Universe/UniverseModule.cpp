#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Spatial/SpatialComponents.h"
#include "Spatial/SpatialProcessors.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<SpatialComponents>();
		world.import<UniverseComponents>();
	}
}