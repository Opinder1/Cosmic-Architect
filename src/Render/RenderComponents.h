#pragma once

#include "RenderInstancer.h"
#include "RenderContext.h"

#include "Commands/TypedCommandBuffer.h"

#include "Util/PerThread.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

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

	struct CContext : Nocopy, Nomove
	{
		PerThread<ThreadContext> threads;
	};

	struct CTransform
	{
		godot::Transform3D transform;
		bool modified = false;
	};

	// This is the scenario that all the children instances of this entity will register to
	struct CScenario
	{
		godot::RID id;
		godot::Vector3i camera_pos; // Updated to the currently enabled cameras position
	};

	// This tag denotes that this entity creates the scenario itself
	struct COwnedScenario {};

	// Relationship tag for setting the base type of the entity with it being a unique instance
	struct CInstance
	{
		godot::RID id;
	};

	// Relationship tag for setting the base type of the entity to a batched type
	struct CMultiInstance {};

	// This entity is a render base which instances will use to define what they render
	struct CBase
	{
		godot::RID id;
	};

	// A tag that denotes that the base is a placeholder cube
	struct CPlaceholderCube {};

	struct CMesh {};

	struct CMeshFile
	{
		godot::StringName path;
	};

	struct CPointMesh : Nocopy
	{
		godot::PackedVector3Array points;
	};

	struct CMultimesh {};

	struct CParticles {};

	struct CParticlesCollision {};

	struct CLight {};

	struct CReflectionProbe {};

	struct CDecal {};

	struct CVoxelGI {};

	struct CLightmap {};

	struct COccluder {};

	struct CVisibilityNotifier {};

	struct CFogVolume {};

	// Used by various lod systems to know where to load visual elements around
	// Is related to the scenario that its in
	struct CCamera {};
}