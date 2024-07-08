#pragma once

#include "CommandQueue.h"

#include "Util/PerThread.h"
#include "Util/Time.h"
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
	struct RenderingServerThreadContext
	{
		CommandBuffer commands;
	};

	struct RenderingServerContext : Nocopy
	{
		godot::RenderingServer* server;
		PerThread<RenderingServerThreadContext> threads;
	};

	struct RenderScenario
	{
		godot::RID id;
	};

	struct OwnedScenario {};

	struct InstanceDirtyFlags
	{
		bool base : 1;
		bool blend_shape_weight : 1;
		bool custom_aabb : 1;
		bool extra_visibility_margin : 1;
		bool ignore_culling : 1;
		bool layer_mask : 1;
		bool pivot_data : 1;
		bool scenario : 1;
		bool surface_override_material : 1;
		bool transform : 1;
		bool visibility_parent : 1;
		bool visible : 1;
	};

	struct RenderInstance
	{
		godot::RID id;
		union
		{
			uint16_t dirty_data = 0;
			InstanceDirtyFlags dirty;
		};
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