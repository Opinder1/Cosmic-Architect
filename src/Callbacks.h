#pragma once

#include "UniverseSimulation.h"

namespace voxel_game
{
	struct UniverseSimulation::Callbacks
	{
		std::vector<godot::Callable> start_local_galaxy;
		std::vector<godot::Callable> start_local_fragment;
		std::vector<godot::Callable> start_remote_galaxy;
		std::vector<godot::Callable> stop_galaxy;
	};
}