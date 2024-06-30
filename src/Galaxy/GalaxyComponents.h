#pragma once

#include "Spatial/SpatialComponents.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/dir_access.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <flecs/flecs.h>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct GalaxyNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};

	struct GalaxyScale : SpatialScale3D
	{

	};

	struct GalaxyComponent
	{
		uint32_t galaxy_texture_index = 0;
	};

	struct GalaxyObjectComponent {};

	struct StarComponent
	{
		godot::Color color;
	};

	struct SimulatedGalaxyComponent : Nocopy
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

	struct GalaxyComponents
	{
		GalaxyComponents(flecs::world& world);
	};
}