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
	// This is a specialised node for a galaxy
	struct GalaxyNode : SpatialNode3D
	{
		std::vector<flecs::entity_t> entities;
	};

	// This is a specialised scale for a galaxy
	struct GalaxyScale : SpatialScale3D
	{

	};

	// This entity is a galaxy which is a 2d square with a galaxy texture
	struct GalaxyComponent
	{
		uint32_t galaxy_texture_index = 0;
	};

	// This is an entity that is a child of a galaxy
	struct GalaxyObjectComponent {};

	// This entity is a star
	struct StarComponent
	{
		godot::Color color;
	};

	// This entity is galaxy that has a specialised spatial world
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