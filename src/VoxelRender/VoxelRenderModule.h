#pragma once

namespace voxel_game::universe
{
	struct Simulation;
}

namespace voxel_game::voxelrender
{
	void Initialize(universe::Simulation& simulation);

	void Uninitialize(universe::Simulation& simulation);

	void Update(universe::Simulation& simulation);

	void WorkerUpdate(universe::Simulation& simulation, size_t index);
}