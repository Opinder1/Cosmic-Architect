#pragma once

#include <flecs/flecs.h>

namespace voxel_game::sim
{
	class ThreadEntityPool;

	ThreadEntityPool& GetPool();

	struct Module
	{
		Module(flecs::world& world);
	};
}