#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	Module::Module(flecs::world& world)
	{
		world.import<Components>();

		world.singleton<Velocity>()
			.add_second<Position>(flecs::With);

		world.system<Position, Velocity>(DEBUG_ONLY("ApplyVelocity"))
			.multi_threaded()
			.each([](Position& position, Velocity& velocity)
		{
			position.position += velocity.velocity;
		});
	}
}