#include "PlayerComponents.h"

namespace voxel_game::player
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CPlayer>();
		world.component<GPlayers>();
	}
}