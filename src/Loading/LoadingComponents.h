#pragma once

#include "EntityLoader.h"

namespace flecs
{
	class world;
}

namespace voxel_game::loading
{
	struct SaveEvent
	{
		EntitySaveData data;
	};

	struct Components
	{
		Components(flecs::world& world);
	};
}