#pragma once

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct RenderInstance
	{
		godot::RID id;
	};

	struct RenderBase {};

	struct RenderMesh
	{
		godot::RID id;
	};

	struct RenderMultiMesh
	{
		godot::PackedByteArray buffer;
	};

	struct RenderMultiInstance {};

	struct RenderScenario
	{
		godot::RID id;
	};

	struct RenderComponents
	{
		RenderComponents(flecs::world& world);
	};
}