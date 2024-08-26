#pragma once

#include "CommandQueue.h"

#include "Util/PerThread.h"
#include "Util/Time.h"
#include "Util/SmallVector.h"
#include "Util/DirtyTracker.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <array>

namespace flecs
{
	struct world;
}

namespace godot
{
	class RenderingServer;
}

namespace voxel_game
{
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
		Invalid,
	};

	struct RenderingServerThreadContext
	{
		CommandBuffer commands;
	};

	struct RenderingServerContext : Nocopy
	{
		RenderingServerContext();
		~RenderingServerContext();

		godot::RenderingServer* server = nullptr;
		PerThread<RenderingServerThreadContext> threads;
		RenderingServerThreadContext main_thread;
	};

	struct InstanceDataType
	{
		// Format for how data is stored for each instance
		InstanceDataFormat format = InstanceDataFormat::Invalid;

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

	using MeshInstancers = robin_hood::unordered_map<flecs::entity_t, MeshTypeInstancer>;

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
		godot::RID id;
	};

	struct ModifyFlags
	{
		ModifyFlags()
		{
			std::memset(this, 0, sizeof(*this));
		}

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

	// A node in the main render transform tree that follows node draw info from its parent
	struct RenderTreeNode : Nocopy
	{
		godot::Transform3D transform;
		godot::Vector3 velocity;
		godot::AABB aabb;
		bool visible = false;

		ModifyFlags modify_flags;

		// Mesh type instancers for meshes that can be made into multimeshes
		MeshInstancers mesh_instancers;
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