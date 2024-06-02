#include "RenderComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderComponents::RenderComponents(flecs::world& world)
	{
		world.module<RenderComponents>();

        world.component<RenderingServerContext>();
        world.component<RenderInstance>();
        world.component<RenderMesh>();
        world.component<RenderScenario>();

        world.set([](RenderingServerContext& context)
        {
            context.server = godot::RenderingServer::get_singleton();
        });

        world.observer<RenderInstance, RenderingServerContext>()
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderInstance& instance, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            instance.id = context.server->instance_create();

            CommandBuffer::AddCommand(commands, "instance_set_base", instance.id, context.server->get_test_cube());
        });

        world.observer<const RenderInstance, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "free_rid", instance.id);
        });

        world.observer<RenderScenario, RenderingServerContext>()
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderScenario& scenario, RenderingServerContext& context)
        {
            scenario.id = context.server->scenario_create();
        });

        world.observer<const RenderScenario, RenderingServerContext>()
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderScenario& scenario, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "free_rid", scenario.id);
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
            .each([](flecs::entity entity, const RenderMesh& mesh, RenderingServerContext& context)
        {
            auto& commands = context.thread_buffers[entity.world().get_stage_id()];

            CommandBuffer::AddCommand(commands, "free_rid", mesh.id);
        });
	}
}