#include "PlayerModule.h"
#include "PlayerComponents.h"

namespace voxel_game::player
{
	entity::Ptr CreateLocalPlayer(universe::Simulation& simulation, entity::Ptr server, const godot::String& name)
	{
		return entity::Ptr{};
	}
}