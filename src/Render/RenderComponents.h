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

	// This is the scenario that all the children instances of this entity will register to
	struct RenderScenario
	{
		godot::RID id;
	};

	// This scenarios lifetime is managed by this entity
	struct OwnedScenario {};

	// This is an instance in the render server which should have a render base set
	struct RenderInstance
	{
		godot::RID id;
	};

	// Flags for instance data that has been modified
	struct RenderInstanceFlags
	{
		RenderInstanceFlags()
		{
			std::memset(this, 0, sizeof(*this));
		}

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

	// This is a relationship which is used to set the render base this render instamnace will draw will draw
	struct RenderBase {};

	// This entity has a mesh which children with a base of this entity will draw
	struct RenderMesh
	{
		godot::RID id;
	};

	// This entity has a multi mesh which children with multi instance will write to
	struct RenderMultiMesh
	{
		godot::PackedByteArray buffer;
	};

	// this entity registers using a multi instance
	struct RenderMultiInstance {};

	// This entity draws a flat square in 3d space
	struct FlatTextureComponent
	{
		uint32_t texture_index = 0;
	};

	struct RenderComponents
	{
		RenderComponents(flecs::world& world);
	};
}