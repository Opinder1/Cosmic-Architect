#pragma once

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::simulation
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);
}