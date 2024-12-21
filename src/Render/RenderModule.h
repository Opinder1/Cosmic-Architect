#pragma once

namespace flecs
{
	struct world;
}

namespace voxel_game::rendering
{
	struct Module
	{
		Module(flecs::world& world);

		void InitTransform(flecs::world& world);

		void InitScenario(flecs::world& world);

		void InitUniqueInstance(flecs::world& world);

		void InitBase(flecs::world& world);

		void InitPlaceholderCube(flecs::world& world);

		void InitMesh(flecs::world& world);
	};
}