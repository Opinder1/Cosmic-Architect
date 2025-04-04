#pragma once

#include <godot_cpp/variant/rid.hpp>

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::rendering
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	bool IsEnabled();
}