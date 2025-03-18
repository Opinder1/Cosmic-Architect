#include "NetworkingModule.h"
#include "NetworkingComponents.h"

namespace voxel_game::network
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();

	}
}