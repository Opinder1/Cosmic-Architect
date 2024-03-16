#pragma once

#include <godot_cpp/variant/rid.hpp>

#include <godot_cpp/variant/transform3d.hpp>

namespace voxel_game
{
	struct Scenario
	{
		godot::RID scenario_id;
	};

	struct Instance
	{
		godot::RID instance_id;
	};

	struct MultimeshInstance {};

	struct Mesh
	{
		godot::RID mesh_id;
	};

	struct Position
	{
		godot::Vector3 position;
	};

	struct Velocity
	{
		godot::Vector3 velocity;
	};
}