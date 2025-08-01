#pragma once

#include <godot_cpp/variant/string.hpp>

namespace voxel_game
{
	struct Simulation;
	struct ThreadContext;
}

namespace voxel_game::simulation
{
	void SetContext(ThreadContext& context);
	ThreadContext& GetContext();

	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	bool IsUnloadDone(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);
}