#pragma once

#include "GalaxyRenderWorld.h"

#include <flecs/flecs.h>

namespace voxel_game::galaxyrender
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// This entity is a galaxy
	struct CWorld {};
}