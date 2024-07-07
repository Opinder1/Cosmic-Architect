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
        world.component<OwnedScenario>();
        world.component<RenderInstance>();
        world.component<RenderMesh>();
        world.component<RenderMultiMesh>();

        world.entity<RenderBase>()
            .add(flecs::Relationship)
            .add(flecs::Traversable)
            .add(flecs::Exclusive);

        world.entity<RenderMultiInstance>()
            .add(flecs::Relationship)
            .add(flecs::Traversable)
            .add(flecs::Exclusive);

        world.set([&world](RenderingServerContext& context)
        {
            context.server = godot::RenderingServer::get_singleton();
        });

        world.system<RenderingServerContext>(DEBUG_ONLY("FlushRenderingServerCommands"))
            .term_at(1).src<RenderingServerContext>()
            .no_readonly()
            .each([](RenderingServerContext& context)
        {
            for (CommandBuffer& thread_buffer : context.thread_buffers)
            {
                CommandQueueServer::get_singleton()->AddCommands(context.server->get_instance_id(), std::move(thread_buffer));
            }
        });

        world.observer<RenderScenario, RenderingServerContext>()
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .term<const OwnedScenario>()
            .each([](RenderScenario& scenario, RenderingServerContext& context)
        {
            scenario.id = context.server->scenario_create();
        });

        world.observer<const RenderScenario, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .term<const OwnedScenario>()
            .each([](const RenderScenario& scenario, RenderingServerContext& context)
        {
            CommandBuffer& commands = context.thread_buffers[0];

            CommandBuffer::AddCommand(commands, "free_rid", scenario.id);
        });

        world.observer<RenderInstance, RenderingServerContext>()
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](RenderInstance& instance, RenderingServerContext& context)
        {
            instance.id = context.server->instance_create();
        });

        world.observer<const RenderInstance, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, RenderingServerContext& context)
        {
            CommandBuffer& commands = context.thread_buffers[0];

            CommandBuffer::AddCommand(commands, "free_rid", instance.id);
        });

        world.observer<RenderMesh, RenderingServerContext>()
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderMesh& mesh, RenderingServerContext& context)
        {
            mesh.id = context.server->mesh_create();
        });

        world.observer<const RenderMesh, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const RenderMesh& mesh, RenderingServerContext& context)
        {
            CommandBuffer& commands = context.thread_buffers[0];

            CommandBuffer::AddCommand(commands, "free_rid", mesh.id);
        });
	}
}