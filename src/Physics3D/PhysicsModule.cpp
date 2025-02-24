#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include "Util/Debug.h"

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

		world.singleton<CVelocity>()
			.add_second<CPosition>(flecs::With);

		world.system<CPosition, CVelocity>(DEBUG_ONLY("ApplyVelocity"))
			.multi_threaded()
			.each([](CPosition& position, CVelocity& velocity)
		{
			position.position += velocity.velocity;
		});
	}
}