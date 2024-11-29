#pragma once

#include "Util/Nocopy.h"
#include "Util/SmallVector.h"
#include "Util/DirtyTracker.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/aabb.hpp>

#include <robin_hood/robin_hood.h>

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game::rendering
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

	// Mesh type instancers for meshes that can be made into multimeshes
	struct MeshInstancers : Nocopy
	{
		robin_hood::unordered_map<flecs::entity_t, MeshTypeInstancer> instances;
	};
}