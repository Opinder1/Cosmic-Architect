#pragma once

#include <godot_cpp/variant/rid.hpp>

namespace voxel_game::universe
{
	struct Simulation;
}

namespace voxel_game::rendering
{
	void Initialize(universe::Simulation& simulation);

	void Uninitialize(universe::Simulation& simulation);

	void Update(universe::Simulation& simulation);

	void WorkerUpdate(universe::Simulation& simulation, size_t index);

	bool IsEnabled();
}