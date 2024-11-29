#pragma once

#include "Util/Nocopy.h"

#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/aabb.hpp>

#include <flecs/flecs.h>

#include <array>
#include <bitset>

namespace voxel_game::rendering
{
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
	};

	struct TreeComponents
	{
		TreeComponents(flecs::world& world);
	};
}