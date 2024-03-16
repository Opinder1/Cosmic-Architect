#pragma once

#include "UniverseSimulation.h"

namespace voxel_game
{
	struct UniverseSimulation::Signals
	{
		Signals() {}

		godot::StringName galaxy_started;
	};
}