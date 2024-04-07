#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ip.hpp>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace voxel_game
{
	struct ScenarioComponent
	{
		godot::RID scenario;
	};

	struct UniverseCameraComponent
	{

	};

	struct UniverseObjectComponent
	{

	};

	struct StarRendererComponent
	{
		godot::RID multimesh;
	};

	struct StarComponent
	{

	};

	struct GalaxyRendererComponent
	{
		godot::RID multimesh;
	};

	struct GalaxyComponent
	{
		godot::Color color;
	};

	struct SimulatedGalaxyComponent
	{
		godot::StringName name;
		godot::StringName path;

		bool networked;
		bool is_remote;
		godot::StringName remote_ip;

		size_t main_seed;
	};
}