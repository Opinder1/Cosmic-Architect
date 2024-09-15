#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::physics
{
	Module::Module(flecs::world& world)
	{
		world.import<Components>();

		world.singleton<Velocity3D>()
			.add_second<Position3D>(flecs::With);

		world.system<Position3D, Velocity3D>(DEBUG_ONLY("ApplyVelocity"))
			.multi_threaded()
			.each([](Position3D& position, Velocity3D& velocity)
		{
			position.position += velocity.velocity;
		});
	}
}