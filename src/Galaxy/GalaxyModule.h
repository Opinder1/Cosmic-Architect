#pragma once

#include "Entity/EntityPoly.h"

namespace godot
{
	class String;
}

namespace voxel_game::universe
{
	struct Simulation;
}

namespace voxel_game::galaxy
{
	void Initialize(universe::Simulation& simulation);

	void Uninitialize(universe::Simulation& simulation);

	void Update(universe::Simulation& simulation);

	void WorkerUpdate(universe::Simulation& simulation, size_t index);

	entity::Ref CreateNewSimulatedGalaxy(universe::Simulation& simulation, const godot::String& path, entity::WRef universe_entity);

	void DestroySimulatedGalaxy(universe::Simulation& simulation, entity::WRef galaxy);
}