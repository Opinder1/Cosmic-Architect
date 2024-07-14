#include "RenderModule.h"
#include "RenderComponents.h"

#include "Physics/PhysicsComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
	RenderModule::RenderModule(flecs::world& world)
	{
		world.module<RenderModule>();

		world.import<RenderComponents>();

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

        world.observer<RenderScenario, RenderingServerContext>(DEBUG_ONLY("AddRenderScenario"))
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .term<const OwnedScenario>()
            .each([](RenderScenario& scenario, RenderingServerContext& context)
        {
            scenario.id = context.server->scenario_create();
        });

        world.observer<const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RemoveRenderScenario"))
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .term<const OwnedScenario>()
            .each([](const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", scenario.id);
        });

        world.observer<RenderInstance, RenderingServerContext>(DEBUG_ONLY("AddRenderInstance"))
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](RenderInstance& instance, RenderingServerContext& context)
        {
            instance.id = context.server->instance_create();
        });

        world.observer<const RenderInstance, RenderingServerContext>(DEBUG_ONLY("RemoveRenderInstance"))
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, const RenderInstance& instance, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", instance.id);
        });

        world.observer<RenderMesh, RenderingServerContext>(DEBUG_ONLY("AddRenderMesh"))
            .event(flecs::OnAdd)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderMesh& mesh, RenderingServerContext& context)
        {
            mesh.id = context.server->mesh_create();
        });

        world.observer<const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RemoveRenderMesh"))
            .event(flecs::OnRemove)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "free_rid", mesh.id);
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetScenario"))
            .event(flecs::OnSet)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "instance_set_scenario", instance.id, scenario.id);
        });

        world.observer<const RenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(2).parent()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "instance_set_scenario", instance.id, godot::RID());
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetMesh"))
            .event(flecs::OnSet)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "instance_set_base", instance.id, mesh.id);
        });

        world.observer<const RenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveMesh"))
            .event(flecs::OnRemove)
            .term_at(2).up<RenderBase>()
            .term_at(3).src<RenderingServerContext>().filter()
            .each([](const RenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            CommandBuffer::AddCommand(thread_context.commands, "instance_set_base", instance.id, godot::RID());
        });

        world.system<RenderInstance, RenderingServerContext>(DEBUG_ONLY("UpdateRenderInstanceTransforms"))
            .multi_threaded()
            .term_at(2).src<RenderingServerContext>()
            .term<const Position3DComponent>().optional()
            .term<const Rotation3DComponent>().optional()
            .term<const Scale3DComponent>().optional()
            .each([](flecs::iter& it, size_t i, RenderInstance& instance, RenderingServerContext& context)
        {
            if (!instance.dirty.transform)
            {
                return;
            }

            instance.dirty.transform = false;

            godot::Transform3D transform;

            if (it.is_set(3))
            {
                auto& position = it.field_at<const Position3DComponent>(3, i);

                transform.set_origin(position.position);
            }

            if (it.is_set(4))
            {
                auto& rotation = it.field_at<const Rotation3DComponent>(4, i);

                transform.rotate(rotation.rotation.get_axis(), rotation.rotation.get_angle());
            }

            if (it.is_set(5))
            {
                auto& scale = it.field_at<const Scale3DComponent>(5, i);

                transform.scale(scale.scale);
            }

            RenderingServerThreadContext& thread_context = context.threads[it.world().get_stage_id()];

            CommandBuffer::AddCommand(thread_context.commands, "instance_set_transform", instance.id, transform);
        });
	}
}