#pragma once

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/ip.hpp>

namespace voxel_game
{
	struct Position
	{
		godot::Vector3 position;
	};

	struct Velocity
	{
		godot::Vector3 velocity;
	};

	struct UniverseScene
	{
		godot::RID scenario;
	};

	struct UniverseCamera
	{

	};

	struct UniverseObject
	{

	};

	struct Galaxy
	{
		godot::Color color;
	};

	struct SimulatedGalaxy
	{
		godot::String name;
		godot::String path;

		bool networked;
		bool is_remote;
		godot::IP remote_ip;

		size_t main_seed;
	};
}