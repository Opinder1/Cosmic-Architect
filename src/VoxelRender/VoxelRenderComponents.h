#pragma once

#include "Util/Poly.h"

#include <godot_cpp/variant/rid.hpp>

#include <flecs/flecs.h>

namespace voxel_game::voxelrender
{
	constexpr const size_t k_max_voxel_types = UINT16_MAX;

	struct Components
	{
		Components(flecs::world& world);
	};

	struct CContext
	{
		int i;
	};

	struct Node
	{
		godot::RID mesh;
		godot::RID mesh_instance;
	};

	struct Scale {};

	struct World
	{
		godot::RID voxel_material;
	};

	struct CWorld {};
}