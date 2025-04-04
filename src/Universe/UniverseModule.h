#pragma once

#include "Entity/EntityComponents.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::universe
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	entity::Ref CreateNewUniverse(Simulation& simulation, const godot::StringName& path);
}