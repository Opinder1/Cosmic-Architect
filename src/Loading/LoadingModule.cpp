#include "LoadingModule.h"
#include "LoadingComponents.h"
#include "ArchiveServer.h"

#include "Simulation/SimulationComponents.h"

namespace voxel_game::loading
{
	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
		world.import<sim::Components>();

		world.ensure<ArchiveServer>().Initialize(world, ArchiveServer::MultiThreaded);

		world.system<ArchiveServer>(DEBUG_ONLY("ProgressArchiveServer"))
			.each([](ArchiveServer& server)
		{
			server.Progress();
		});
	}
}