#include "SimulationModule.h"
#include "SimulationComponents.h"

#include <flecs/flecs.h>

namespace voxel_game
{
	SimulationModule::SimulationModule(flecs::world& world)
	{
		world.module<SimulationModule>();

		world.import<SimulationComponents>();

		world.system<SimulationTime>("WorldUpdateTime")
			.kind(flecs::OnUpdate)
			.term_at(1).src<SimulationTime>()
			.each([](SimulationTime& world_time)
		{
			world_time.frame_index++;
			world_time.frame_start = Clock::now();
		});

		world.system<EntityThreadCommands>("ProcessEntityThreadCommands")
			.kind(flecs::OnUpdate)
			.term_at(1).src<EntityThreadCommands>()
			.each([&world](EntityThreadCommands& loader_context)
		{
			for (EntityThreadCommands::ThreadData& thread : loader_context.threads)
			{
				for (EntityCreateCommand command : thread.create_commands)
				{
					world.entity().add(command.prefab, flecs::Prefab);
					world.entity().add(command.parent, flecs::Parent);
				}

				thread.create_commands.clear();

				for (EntityDestroyCommand command : thread.destroy_commands)
				{
					world.entity(command.entity).destruct();
				}

				thread.destroy_commands.clear();
			}
		});
	}
}