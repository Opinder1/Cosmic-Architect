#include "UniverseModule.h"
#include "UniverseComponents.h"

#include "Physics/PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseModule::UniverseModule(flecs::world& world)
	{
		world.module<UniverseModule>("UniverseModule");

		world.import<UniverseComponents>();

		world.prefab("Universe");
	}
}