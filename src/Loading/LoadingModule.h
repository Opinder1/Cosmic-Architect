#pragma once

namespace flecs
{
	class world;
	struct entity;
}

namespace voxel_game::loading
{
	struct Module
	{
		Module(flecs::world& world);
	};

	void SaveEntity(flecs::entity entity);
}