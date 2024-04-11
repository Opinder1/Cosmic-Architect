#pragma once

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct UniverseComponent {};

	struct UniverseCameraComponent {};

	struct UniverseObjectComponent {};

	struct GalaxyComponent
	{
		godot::Color color;
	};

	struct GalaxyObjectComponent {};

	struct StarComponent
	{
		godot::Color color;
	};

	struct SimulatedGalaxyComponent
	{
		godot::StringName name;
		godot::StringName path;
		godot::StringName fragment_type;

		bool networked = false;
		bool is_remote = false;
		godot::StringName remote_ip;

		size_t main_seed = 0;

		godot::Ref<godot::DirAccess> m_directory;
	};

	struct UniverseComponents
	{
		UniverseComponents(flecs::world& world);
	};
}