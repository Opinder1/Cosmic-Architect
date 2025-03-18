#include "PlayerModule.h"
#include "PlayerComponents.h"

namespace voxel_game::player
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
	}

	flecs::entity_t CreateLocalPlayer(flecs::world& world, flecs::entity_t server, const godot::String& name)
	{
		flecs::entity entity = world.scope(server).entity();

		entity.add<CPlayer>();

		return entity;
	}
}