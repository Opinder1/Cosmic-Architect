#include "UniverseComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	UniverseComponents::UniverseComponents(flecs::world& world)
	{
		world.module<UniverseComponents>();

		world.component<UniverseComponent>();
		world.component<UniverseObjectComponent>();

		world.component<UniverseComponent>()
			.add_second<SpatialWorld3DComponent>(flecs::With);

		world.component<UniverseObjectComponent>()
			.add_second<UniverseComponent>(flecs::OneOf)
			.add_second<SpatialEntity3DComponent>(flecs::With);
	}
}