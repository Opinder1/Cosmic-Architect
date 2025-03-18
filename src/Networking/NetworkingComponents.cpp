#include "NetworkingComponents.h"

namespace voxel_game::network
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CCertificate>();
		world.component<CServer>();
		world.component<CPeer>();
	}
}