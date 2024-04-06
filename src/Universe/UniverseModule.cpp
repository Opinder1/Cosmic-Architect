#include "UniverseModule.h"
#include "UniverseComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.component<UniverseScene>();
		world.component<UniverseCamera>();
		world.component<UniverseObject>();
	}
}