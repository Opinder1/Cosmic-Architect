#pragma once

#include "Util/Poly.h"

#include <godot_cpp/variant/color.hpp>

#include <flecs/flecs.h>

#include <vector>

namespace voxel_game::galaxy
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// This is a specialised node for a galaxy
	struct Node
	{
		std::vector<flecs::entity_t> stars;
	};

	// This is a specialised scale for a galaxy
	struct Scale {};

	// This entity is a galaxy
	struct CWorld {};

	// This entity is a star
	struct CStar
	{
		godot::Color color;
	};
}