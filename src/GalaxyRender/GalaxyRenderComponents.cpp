#include "GalaxyRenderComponents.h"

namespace voxel_game::galaxyrender
{
	Components::Components(flecs::world& world)
	{
		world.module<Components>();

		world.component<CWorld>();
	}
}