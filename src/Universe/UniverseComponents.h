#pragma once

#include "Simulation/Config.h"

namespace voxel_game::universe
{
	// This entitiy is a universe which has a specialised spatial world
	struct CUniverse
	{
		godot::String path;

		simulation::Config config;
		Clock::time_point last_config_save;
	};
}