#pragma once

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::spatial3d
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	void ParallelWorldUpdate(Simulation& simulation, size_t index);

	void ParallelScaleUpdate(Simulation& simulation, size_t index);
}