#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    Components::Components(flecs::world& world)
	{
		world.module<Components>();

        world.component<ServerContext>()
            .on_remove([](ServerContext& context)
        {
            godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();
            CommandQueueServer* cqserver = CommandQueueServer::get_singleton();

            for (ServerThreadContext& thread_context : context.threads)
            {
                cqserver->AddCommands(rserver->get_instance_id(), std::move(thread_context.commands));
            }
        });

        world.component<TreeNode>();

        world.component<Scenario>();
        world.component<OwnedScenario>();

        world.component<Instance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.component<UniqueInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive)
            .add_second<Instance>(flecs::With);

        world.component<Base>();

        world.component<PlaceholderCube>()
            .add_second<Base>(flecs::With);

        world.component<Mesh>()
            .add_second<Base>(flecs::With);
	}
}