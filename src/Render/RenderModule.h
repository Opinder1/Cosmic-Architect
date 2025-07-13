#pragma once

#include "RenderAllocator.h"

#include "Simulation/SimulationModule.h"

#include "UniverseSimulation.h"

#include <godot_cpp/variant/rid.hpp>

namespace voxel_game::rendering
{
	template<auto Command, class... Args>
	void AddCommand(Args&&... args)
	{
		simulation::GetContext().commands.AddCommand<Command>(args...);
	}

	inline godot::RID AllocRID(RIDType type)
	{
		return simulation::GetContext().allocator.GetRID(type);
	}

	// Module functions
	void Initialize(Simulation& simulation);
	void Uninitialize(Simulation& simulation);
	void Update(Simulation& simulation);
	void WorkerUpdate(Simulation& simulation, size_t index);

	bool IsEnabled();
}