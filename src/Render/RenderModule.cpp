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

        world.singleton<RenderInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive)
            .add(flecs::Traversable);

        world.singleton<UniqueRenderInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive)
            .add(flecs::Traversable);

        world.set([&world](RenderingServerContext& context)
        {
            context.server = godot::RenderingServer::get_singleton();
        });

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<RenderingServerContext>(DEBUG_ONLY("FlushRenderingServerCommands"))
            .term_at(0).src<RenderingServerContext>()
            .immediate()
            .each([](RenderingServerContext& context)
        {
            for (RenderingServerThreadContext& thread_context : context.threads)
            {
                CommandQueueServer::get_singleton()->AddCommands(context.server->get_instance_id(), std::move(thread_context.commands));
            }
        });

        world.observer<RenderScenario, const RenderingServerContext>(DEBUG_ONLY("AddRenderScenario"))
            .event(flecs::OnAdd)
            .with<const OwnedRenderScenario>()
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderScenario& scenario, const RenderingServerContext& context)
        {
            scenario.id = context.server->scenario_create();
            
            entity.modified<RenderScenario>();
        });

        world.observer<const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RemoveRenderScenario"))
            .event(flecs::OnRemove)
            .with<const OwnedRenderScenario>()
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            thread_context.commands.AddCommand("free_rid", scenario.id);
        });

        world.observer<RenderMesh, const RenderingServerContext>(DEBUG_ONLY("AddRenderMesh"))
            .event(flecs::OnAdd)
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, RenderMesh& mesh, const RenderingServerContext& context)
        {
            //mesh.id = context.server->mesh_create();
            mesh.id = context.server->get_test_cube();

            entity.modified<RenderMesh>();
        });

        world.observer<const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RemoveRenderMesh"))
            .event(flecs::OnRemove)
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            thread_context.commands.AddCommand("free_rid", mesh.id);
        });

        world.observer<UniqueRenderInstance, const RenderingServerContext>(DEBUG_ONLY("AddUniqueRenderInstance"))
            .event(flecs::OnAdd)
            .term_at(0).self().second(flecs::Wildcard)
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](flecs::entity entity, UniqueRenderInstance& instance, const RenderingServerContext& context)
        {
            instance.id = context.server->instance_create();

            entity.modified<UniqueRenderInstance>();
        });

        world.observer<const UniqueRenderInstance, RenderingServerContext>(DEBUG_ONLY("RemoveUniqueRenderInstance"))
            .event(flecs::OnRemove)
            .term_at(1).src<RenderingServerContext>().filter()
            .each([](const UniqueRenderInstance& instance, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            thread_context.commands.AddCommand("free_rid", instance.id);
        });

        world.observer<const UniqueRenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetScenario"))
            .event(flecs::OnSet)
            .term_at(0).self().second(flecs::Wildcard)
            .term_at(1).up(flecs::ChildOf).filter()
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const UniqueRenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            thread_context.commands.AddCommand("instance_set_scenario", instance.id, scenario.id);
        });

        world.observer<const UniqueRenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0).self().filter()
            .term_at(1).up(flecs::ChildOf)
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const UniqueRenderInstance&, const RenderScenario&, RenderingServerContext&)
        {
            DEBUG_PRINT_ERROR("Removed a render scenario before instances in it were removed");
        });

        world.observer<const UniqueRenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetMesh"))
            .event(flecs::OnSet)
            .term_at(0).self().second("$Base")
            .term_at(1).src("$Base")
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const UniqueRenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[0];

            thread_context.commands.AddCommand("instance_set_base", instance.id, mesh.id);
        });

        world.observer<const UniqueRenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveMesh"))
            .event(flecs::OnRemove)
            .term_at(0).self().filter()
            .term_at(1).up<UniqueRenderInstance>()
            .term_at(2).src<RenderingServerContext>().filter()
            .each([](const UniqueRenderInstance&, const RenderMesh&, RenderingServerContext&)
        {
            DEBUG_PRINT_ERROR("Removed a render base before instances in it were removed");
        });

        world.system<UniqueRenderInstance, const Position3DComponent*, const Rotation3DComponent*, const Scale3DComponent*, RenderingServerContext>(DEBUG_ONLY("UpdateRenderInstanceTransforms"))
            .multi_threaded()
            .term_at(0).self().second(flecs::Wildcard)
            .term_at(4).src<RenderingServerContext>()
            .each([](flecs::entity entity, UniqueRenderInstance& instance, const Position3DComponent* position, const Rotation3DComponent* rotation, const Scale3DComponent* scale, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            if (!instance.transform || thread_context.commands.NumCommands() > k_max_thread_render_commands)
            {
                return;
            }

            instance.transform = false;

            godot::Transform3D transform;

            if (scale != nullptr)
            {
                transform.scale(scale->scale);
            }

            if (rotation != nullptr)
            {
                transform.rotate(rotation->rotation.get_axis(), rotation->rotation.get_angle());
            }

            if (position != nullptr)
            {
                transform.set_origin(position->position);
            }

            thread_context.commands.AddCommand("instance_set_transform", instance.id, transform);
        });
	}
}