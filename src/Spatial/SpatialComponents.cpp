#include "SpatialComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SpatialComponents::SpatialComponents(flecs::world& world)
	{
		world.module<SpatialComponents>();

		// Components
		world.component<SpatialEntity3DComponent>();
		world.component<SpatialWorld3DComponent>();
		world.component<SpatialScale3DWorkerComponent>();
		world.component<SpatialScale3DWorkerComponent>();
		world.component<SpatialRegion3DWorkerComponent>();
		world.component<SpatialLoader3DComponent>();

		world.entity<SpatialWorldMultithreadPhase>();
	}
}