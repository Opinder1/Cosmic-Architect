#include "PlayerModule.h"
#include "PlayerComponents.h"

namespace voxel_game::player
{
	entity::Ref CreateLocalPlayer(universe::Simulation& simulation, entity::WRef server, const godot::String& name)
	{
		return entity::Ref{};
	}
}