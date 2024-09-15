#include "SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::spatial
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		// Components
		world.component<Scale3DWorker>();
		world.component<Scale3DWorker>();
		world.component<Region3DWorker>();
		world.component<Loader3D>();
		world.component<World3D>();
		world.component<Entity3D>();

		world.entity<WorldMultithreadPhase>();
	}
}