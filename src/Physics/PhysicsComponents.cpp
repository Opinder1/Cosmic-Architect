#include "PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	PhysicsComponents::PhysicsComponents(flecs::world& world)
	{
		world.module<PhysicsComponents>();

		world.component<Position3DComponent>();
		world.component<Velocity3DComponent>();
		world.component<MassComponent>();
		world.component<BouncinessComponent>();
		world.component<RoughnessComponent>();
		world.component<AABBComponent>();
		world.component<Box3DComponent>();
		world.component<SphereComponent>();
	}
}