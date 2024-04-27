#include "UniverseComponents.h"
#include "Spatial/SpatialComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseComponents::UniverseComponents(flecs::world& world)
	{
		world.module<UniverseComponents>();

		world.import<SpatialComponents>();

		world.component<UniverseComponent>();
		world.component<UniverseObjectComponent>();

		world.component<UniverseObjectComponent>().add_second<UniverseComponent>(flecs::OneOf);
	}
}