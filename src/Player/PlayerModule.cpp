#include "PlayerModule.h"
#include "Player.h"

namespace voxel_game::player
{
	entity::Ref CreateLocalPlayer(Simulation& simulation, entity::WRef server, const godot::String& name)
	{
		return entity::Ref{};
	}
}