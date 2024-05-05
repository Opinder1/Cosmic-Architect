#include "GodotComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	GodotComponents::GodotComponents(flecs::world& world)
	{
		world.module<GodotComponents>();

		world.component<SignalsComponent>();
	}
}