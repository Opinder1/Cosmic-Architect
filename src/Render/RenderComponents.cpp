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
        world.component<RenderBase>();
        world.component<RenderMesh>();
        world.component<RenderMultiInstance>();
        world.component<RenderMultiMesh>();
        world.component<FlatTextureComponent>();

        world.singleton<RenderBase>()
            .add(flecs::Relationship)
            .add(flecs::Traversable)
            .add(flecs::Exclusive);

        world.singleton<RenderMultiInstance>()
            .add(flecs::Relationship)
            .add(flecs::Traversable)
            .add(flecs::Exclusive);

        world.set([&world](RenderingServerContext& context)
        {
            context.server = godot::RenderingServer::get_singleton();
        });

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<RenderingServerContext>(DEBUG_ONLY("FlushRenderingServerCommands"))
            .term_at(1).src<RenderingServerContext>()
            .no_readonly()
            .each([](RenderingServerContext& context)
        {
            for (RenderingServerThreadContext& thread_context : context.threads)
            {
                CommandQueueServer::get_singleton()->AddCommands(context.server->get_instance_id(), std::move(thread_context.commands));
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
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", scenario.id);
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
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", instance.id);
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
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", mesh.id);
        });
	}
}