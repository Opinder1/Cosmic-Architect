#include "PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::physics
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<Position3D>();
		world.component<Velocity3D>();
		world.component<Rotation3D>();
		world.component<Scale3D>();
		world.component<Mass>();
		world.component<Bounciness>();
		world.component<Roughness>();
		world.component<AABB>();
		world.component<Box3D>();
		world.component<Sphere>();
	}
}