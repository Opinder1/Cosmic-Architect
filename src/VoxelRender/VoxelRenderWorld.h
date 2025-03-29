#pragma once

#include "Util/Poly.h"

#include <godot_cpp/variant/rid.hpp>

namespace voxel_game::voxelrender
{
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
}