#pragma once

#include <flecs/flecs.h>

namespace voxel_game::loading
{
	struct Module
	{
		Module(flecs::world& world);
	};

	void LoadEntity(flecs::world& world, flecs::entity_t entity);

	void SaveEntity(flecs::world& world, flecs::entity_t entity);
}