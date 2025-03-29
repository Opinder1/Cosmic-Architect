#pragma once

#include "Entity/EntityPoly.h"

#include <godot_cpp/variant/string.hpp>

namespace voxel_game::universe
{
	struct Simulation;
}

namespace voxel_game::player
{
	void Initialize(universe::Simulation& simulation);

	void Uninitialize(universe::Simulation& simulation);

	void Update(universe::Simulation& simulation);

	void WorkerUpdate(universe::Simulation& simulation, size_t index);

	entity::Ptr CreateLocalPlayer(universe::Simulation& simulation, entity::Ptr server, const godot::String& name);

	entity::Ptr CreatePlayerAvatar(universe::Simulation& simulation, entity::Ptr player);
}