#pragma once

#include "EntityLoader.h"

#include <optional>
#include <cstdint>

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	struct SaveEvent {};

	struct Components
	{
		Components(flecs::world& world);
	};
}