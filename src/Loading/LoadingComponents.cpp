#include "LoadingComponents.h"

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	Components::Components(flecs::world& world)
	{
		world.component<Loadable>();
	}
}