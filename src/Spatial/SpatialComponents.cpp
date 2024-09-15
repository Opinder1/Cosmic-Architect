#include "SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<Entity3DComponent>();
		world.component<World3DComponent>();
		world.component<Scale3DWorkerComponent>();
		world.component<Scale3DWorkerComponent>();
		world.component<Region3DWorkerComponent>();
		world.component<Loader3DComponent>();

		world.entity<WorldMultithreadPhase>();
	}
}