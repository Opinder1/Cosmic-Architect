#include "PhysicsComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CPosition>();
		world.component<CVelocity>();
		world.component<CRotation>();
		world.component<CScale>();
		world.component<CMass>();
		world.component<CBounciness>();
		world.component<CRoughness>();
		world.component<CAABB>();
		world.component<CBox>();
		world.component<CSphere>();
	}
}