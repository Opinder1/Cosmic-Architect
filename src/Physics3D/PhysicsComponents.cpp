#include "PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<Position>();
		world.component<Velocity>();
		world.component<Rotation>();
		world.component<Scale>();
		world.component<Mass>();
		world.component<Bounciness>();
		world.component<Roughness>();
		world.component<AABB>();
		world.component<Box>();
		world.component<Sphere>();
	}
}