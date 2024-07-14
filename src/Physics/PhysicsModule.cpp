#include "PhysicsModule.h"
#include "PhysicsComponents.h"

#include "Util/Debug.h"

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

		world.system<Position3DComponent, Velocity3DComponent>(DEBUG_ONLY("ApplyVelocity"))
			.multi_threaded()
			.each([](Position3DComponent& position, Velocity3DComponent& velocity)
		{
			position.position += velocity.velocity;
		});
	}
}