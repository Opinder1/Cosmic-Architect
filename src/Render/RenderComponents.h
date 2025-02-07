#pragma once

#include "RenderAllocator.h"
#include "RenderInstancer.h"

#include "Commands/TypedCommandBuffer.h"

#include "Util/PerThread.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

#include <array>
#include <bitset>

namespace voxel_game::rendering
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct ModifyFlag
	{
		enum : uint8_t
		{
			BlendShapeWeight,
			CustomAABB,
			ExtraVisibilityMargin,
			IgnoreCulling,
			LayerMask,
			PivotData,
			Scenario,
			SurfaceOverrideMaterial,
			Transform,
			VisibilityParent,
			Visible,
			Count
		};
	};

	using ModifyFlags = std::bitset<ModifyFlag::Count>;

	struct ThreadContext
	{
		// TODO Make into template with <godot::RenderingServer>
		TypedCommandBuffer commands;

		Allocator material_allocator{ AllocateType::Material };
		Allocator mesh_allocator{ AllocateType::Mesh };
		Allocator instance_allocator{ AllocateType::Instance };
	};

	struct ServerContext : Nocopy
	{
		ThreadContext main_thread;
		PerThread<ThreadContext> threads;
	};

	struct Transform
	{
		godot::Transform3D transform;
		bool modified = false;
	};

	// This is the scenario that all the children instances of this entity will register to
	struct Scenario
	{
		godot::RID id;
		godot::Vector3i camera_pos; // Updated to the currently enabled cameras position
	};

	// This tag denotes that this entity creates the scenario itself
	struct OwnedScenario {};

	// Relationship tag for setting the base type of the entity with it being a unique instance
	struct Instance
	{
		godot::RID id;
	};

	// Relationship tag for setting the base type of the entity to a batched type
	struct MultiInstance {};

	// This entity is a render base which instances will use to define what they render
	struct Base
	{
		godot::RID id;
	};

	// A tag that denotes that the base is a placeholder cube
	struct PlaceholderCube {};

	struct Mesh {};

	struct MeshFile
	{
		godot::StringName path;
	};

	struct PointMesh : Nocopy
	{
		godot::PackedVector3Array points;
	};

	struct Multimesh {};

	struct Particles {};

	struct ParticlesCollision {};

	struct Light {};

	struct ReflectionProbe {};

	struct Decal {};

	struct VoxelGI {};

	struct Lightmap {};

	struct Occluder {};

	struct VisibilityNotifier {};

	struct FogVolume {};

	// Used by various lod systems to know where to load visual elements around
	// Is related to the scenario that its in
	struct Camera {};
}