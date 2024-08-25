#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderComponents::RenderComponents(flecs::world& world)
	{
		world.module<RenderComponents>();

        world.component<RenderingServerContext>();

        world.component<RenderScenario>();
        world.component<OwnedRenderScenario>();

        world.component<RenderTreeNode>();

        world.component<RenderInstance>();
        world.component<UniqueRenderInstance>();

        world.component<RenderBase>();
        world.component<RenderMesh>();
	}
    
    RenderingServerContext::RenderingServerContext()
    {
        server = godot::RenderingServer::get_singleton();
    }

    RenderingServerContext::~RenderingServerContext()
    {
        CommandQueueServer* command_queue_server = CommandQueueServer::get_singleton();

        for (RenderingServerThreadContext& thread_context : threads)
        {
            command_queue_server->AddCommands(server->get_instance_id(), std::move(thread_context.commands));
        }
    }
}