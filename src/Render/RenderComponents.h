#pragma once

#include "RenderInstancer.h"
#include "RenderContext.h"

#include "Commands/TypedCommandBuffer.h"

#include "Util/PerThread.h"
#include "Util/Nocopy.h"

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <bitset>

namespace voxel_game::rendering
{
	struct CContext : Nocopy, Nomove
	{
		godot::RID scenario;

		PerThread<ThreadContext> threads;
	};

	struct CTransform
	{
		godot::Transform3D transform;
		bool modified = false;
	};
	
	struct CScenario
	{
		godot::RID id;
	};

	struct CInstance
	{
		godot::RID id;
	};

	struct CInstancer
	{
		Instancer instancer;
	};

	// This entity is a render base which instances will use to define what they render
	struct CBase
	{
		godot::RID id;
	};

	// A tag that denotes that the base is a placeholder cube
	struct CPlaceholderCube {};

	struct CMesh {};

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

	struct CFogVolume {};
}