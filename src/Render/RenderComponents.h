#pragma once

#include "RenderAllocator.h"
#include "RenderInstancer.h"

#include "Util/CommandQueue.h"
#include "Util/Time.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

#include <array>
#include <bitset>

namespace flecs
{
	struct world;
}

namespace godot
{
	class RenderingServer;
}

namespace voxel_game::rendering
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct ServerThreadContext
	{
		CommandBuffer commands;
	};

	struct ServerContext : Nocopy
	{
		ServerThreadContext main_thread;
		PerThread<ServerThreadContext> threads;

		Allocator allocator;
	};

	struct ModifyFlags
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

	// A node in the main render transform tree that follows node draw info from its parent
	struct TreeNode : Nocopy
	{
		godot::Transform3D transform;
		godot::Vector3 velocity;
		godot::AABB aabb;
		bool visible = false;

		std::bitset<ModifyFlags::Count> modify_flags;

		// Mesh type instancers for meshes that can be made into multimeshes
		MeshInstancers mesh_instancers;
	};

	// This is the scenario that all the children instances of this entity will register to
	struct Scenario
	{
		godot::RID id;
	};

	// This tag denotes that this entity creates the scenario itself
	struct OwnedScenario {};

	// Relationship tag for setting the base type of the entity
	struct Instance {};

	// Relationship tag for setting the base type of the entity with it being a unique instance
	struct UniqueInstance
	{
		godot::RID id;
	};

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
}