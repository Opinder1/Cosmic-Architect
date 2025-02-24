#pragma once

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
	struct ThreadContext;

	ThreadContext& GetContext();

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

#define ADD_RENDER_CMD(command, ...) rendering::GetContext().commands.AddCommand<&godot::RenderingServer::command>(__VA_ARGS__)

#define ALLOC_RENDER_RID(type) rendering::GetContext().allocator.GetRID(rendering::AllocateType::type)