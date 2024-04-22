#include "UniverseModule.h"
#include "UniverseComponents.h"
#include "GalaxyComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();
		world.import<GalaxyComponents>();

		world.prefab("Universe");
	}
}