#pragma once

#include "Entity/EntityPoly.h"

#include <godot_cpp/variant/string.hpp>

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::player
{
	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);

	void WorkerUpdate(Simulation& simulation, size_t index);

	entity::Ref CreateLocalPlayer(Simulation& simulation, entity::WRef server, const godot::String& name);

	entity::Ref CreatePlayerAvatar(Simulation& simulation, entity::WRef player);
}