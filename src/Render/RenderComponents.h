#pragma once

#include "CommandQueue.h"

#include "Util/PerThread.h"
#include "Util/Time.h"
#include "Util/SmallVector.h"
#include "Util/DirtyTracker.h"
#include "Util/Nocopy.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	constexpr const size_t k_max_thread_render_commands = 1024;

	enum class InstanceDataFormat
	{
		Position12Bit, // 12 bit position (4 bit int)
		Position24Bit, // 24 bit position (8 bit int)
		Position48Bit, // 48 bit position (16 bit float)
		Position96Bit, // 96 bit position (32 bit float)
		PositionRotation112bit, // 48 bit position (16 bit float) 64 bit rotation (16 bit float)
		PositionRotation224bit, // 96 bit position (32 bit float) 128 bit rotation (32 bit float)
		PositionRotationVelocity160bit, // 48 bit position (16 bit float) 64 bit rotation (16 bit float) 48 bit velocity (16 bit float)
		PositionRotationVelocity320bit, // 96 bit position (32 bit float) 128 bit rotation (32 bit float) 96 bit velocity (32 bit float)
		Transform2D,
		Transform2DColor1,
		Transform2DColor2,
		Transform2DColor3,
		Transform3D,
		Transform3DColor1,
		Transform3DColor2,
		Transform3DColor3,
	};

	struct RenderingServerThreadContext
	{
		CommandBuffer commands;
	};

	struct RenderingServerContext : Nocopy
	{
		godot::RenderingServer* server;
		PerThread<RenderingServerThreadContext> threads;
	};

	struct InstanceDataType
	{
		// Format for how data is stored for each instance
		InstanceDataFormat format;

		// The region size for each block. If 0 then use just one region
		size_t block_region_size = 0;
	};

	// A block of instances of a mesh type
	struct MeshTypeInstancerBlock : Nocopy
	{
		godot::RID multimesh_id;
		godot::AABB aabb;
		
		DirtyRangeTracker change_tracker;

		std::vector<size_t> instances;
		std::vector<unsigned char> instance_data;
	};

	// A type of mesh instance that is rendered.
	struct MeshTypeInstancer : Nocopy
	{
		godot::RID instance_id;
		godot::RID base_id;

		// Info about how the type is stored in the instance data
		InstanceDataType type;

		// Can have one or many render blocks
		GrowingSmallVector<MeshTypeInstancerBlock, 1> blocks;
	};

	// A node in the main render transform tree.
	struct RenderTreeNode : Nocopy
	{
		godot::Transform3D transform;
		godot::AABB aabb;
		bool visible;

		// Mesh type instancers for meshes that can be made into multimeshes
		robin_hood::unordered_map<flecs::entity_t, MeshTypeInstancer> base_types;

		// Standalone instances for objects that there are only one of or not meshes
		robin_hood::unordered_map<flecs::entity_t, godot::RID> instances;
	};

	// This is the scenario that all the children instances of this entity will register to
	struct RenderScenario
	{
		godot::RID id;
	};
	
	// This tag denotes that this entity creates the scenario itself
	struct OwnedRenderScenario {};

	// Relationship tag for setting the base type of the entity
	struct RenderInstance {};

	// Relationship tag for setting the base type of the entity with it being a unique instance
	struct UniqueRenderInstance
	{
		UniqueRenderInstance()
		{
			std::memset(this, 0, sizeof(*this));
		}

		godot::RID id;

		// Flags for instance data that has been modified
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

	// A tag that denotes that the entity is a render base type
	struct RenderBase {};

	// Set the mesh id for the mesh entity
	struct RenderMesh
	{
		godot::RID id;
	};

	struct RenderComponents
	{
		RenderComponents(flecs::world& world);
	};
}