#pragma once

#include "Entity/EntityComponents.h"

namespace voxel_game::universe
{
	struct Simulation;

	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	entity::Ptr CreateNewUniverse(Simulation& simulation, const godot::StringName& path);
}