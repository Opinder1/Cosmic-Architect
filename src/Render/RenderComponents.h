#pragma once

#include "CommandQueue.h"

#include "Util/PerThread.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <array>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct RenderingThreadData
	{
		CommandBuffer commands;
	};

	struct RenderingServerContext : Nocopy
	{
		godot::RenderingServer* server;
		PerThread<RenderingThreadData> threads;
	};

	struct RenderScenario
	{
		godot::RID id;
	};

	struct OwnedScenario {};

	struct RenderInstance
	{
		godot::RID id;
		bool dirty = false;
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

	struct RenderComponents
	{
		RenderComponents(flecs::world& world);
	};
}