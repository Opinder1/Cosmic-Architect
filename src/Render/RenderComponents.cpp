#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game::rendering
{
    Components::Components(flecs::world& world)
	{
		world.module<Components>();

        world.component<ServerContext>();

        world.component<TreeNode>();

        world.component<Scenario>();
        world.component<OwnedScenario>();

        world.component<Instance>();
        world.component<UniqueInstance>();

        world.component<Base>();
        world.component<PlaceholderCube>();
        world.component<Mesh>();
	}
    
    ServerContext::ServerContext()
    {
        server = godot::RenderingServer::get_singleton();
    }

    ServerContext::~ServerContext()
    {
        CommandQueueServer* command_queue_server = CommandQueueServer::get_singleton();

        for (ServerThreadContext& thread_context : threads)
        {
            command_queue_server->AddCommands(server->get_instance_id(), std::move(thread_context.commands));
        }
    }
}