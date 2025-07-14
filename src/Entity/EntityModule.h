#pragma once

#include "EntityPoly.h"

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::entity
{
#if defined(DEBUG_ENABLED)
	void SetDebugName(Simulation& simulation, entity::WRef entity, const godot::String& name);
#endif
}