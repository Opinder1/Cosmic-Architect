#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	PhysicsModule::PhysicsModule(flecs::world& world)
	{
		world.import<PhysicsComponents>();

		world.singleton<Velocity3DComponent>()
			.add_second<Position3DComponent>(flecs::With);

		world.singleton<Scale3DComponent>()
			.add_second<Position3DComponent>(flecs::With);
	}
}