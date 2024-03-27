#include "UniverseModule.h"
#include "Components.h"
#include "Voxel.h"

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