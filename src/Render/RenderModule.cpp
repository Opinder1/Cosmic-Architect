#include "RenderModule.h"
#include "RenderComponents.h"

#include "Physics/PhysicsComponents.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

namespace voxel_game
{
    const godot::Transform3D k_invisible_transform{ godot::Basis(), godot::Vector3(FLT_MAX, FLT_MAX, FLT_MAX) };

	RenderModule::RenderModule(flecs::world& world)
	{
		world.module<RenderModule>();

		world.import<RenderComponents>();

        world.singleton<RenderInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.singleton<UniqueRenderInstance>()
            .add(flecs::Relationship)
            .add(flecs::Exclusive);

        world.add<RenderingServerContext>();

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<RenderingServerContext>(DEBUG_ONLY("FrameFlushRenderingCommands"))
            .immediate()
            .each([](RenderingServerContext& context)
        {
            CommandQueueServer* command_queue_server = CommandQueueServer::get_singleton();

            uint64_t server_instance = context.server->get_instance_id();

            for (RenderingServerThreadContext& thread_context : context.threads)
            {
                command_queue_server->AddCommands(server_instance, std::move(thread_context.commands));
            }

            // Do main thread commands last as they may include freeing of rids
            command_queue_server->AddCommands(server_instance, std::move(context.main_thread.commands));
        });

        world.observer<RenderScenario, const RenderingServerContext>(DEBUG_ONLY("AddRenderScenario"))
            .event(flecs::OnAdd)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .with<const OwnedRenderScenario>().self()
            .each([](flecs::entity entity, RenderScenario& scenario, const RenderingServerContext& context)
        {
            scenario.id = context.server->scenario_create();
            
            entity.modified<RenderScenario>();
        });

        world.observer<const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RemoveRenderScenario"))
            .event(flecs::OnRemove)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .with<const OwnedRenderScenario>().self()
            .each([](const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand("free_rid", scenario.id);
        });

        world.observer<RenderMesh, const RenderingServerContext>(DEBUG_ONLY("AddRenderMesh"))
            .event(flecs::OnAdd)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .each([](flecs::entity entity, RenderMesh& mesh, const RenderingServerContext& context)
        {
            //mesh.id = context.server->mesh_create();
            mesh.id = context.server->get_test_cube();

            entity.modified<RenderMesh>();
        });

        world.observer<const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RemoveRenderMesh"))
            .event(flecs::OnRemove)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .each([](const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(mesh.id != godot::RID(), "Mesh should be valid");

            thread_context.commands.AddCommand("free_rid", mesh.id);
        });

        world.observer<UniqueRenderInstance, const RenderingServerContext>(DEBUG_ONLY("AddUniqueRenderInstance"))
            .event(flecs::OnAdd)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](flecs::iter& it, size_t i, UniqueRenderInstance& instance, const RenderingServerContext& context)
        {
            instance.id = context.server->instance_create();

            it.entity(i).modified(it.pair(0));
        });

        world.observer<const UniqueRenderInstance, RenderingServerContext>(DEBUG_ONLY("RemoveUniqueRenderInstance"))
            .event(flecs::OnRemove)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](const UniqueRenderInstance& instance, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            thread_context.commands.AddCommand("free_rid", instance.id);
        });

        world.observer<const UniqueRenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetScenario"))
            .event(flecs::OnSet)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).up(flecs::ChildOf).filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueRenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand("instance_set_transform", instance.id, k_invisible_transform); // Fake invisibility until we set transform
            thread_context.commands.AddCommand("instance_set_scenario", instance.id, scenario.id);
        });

#if defined(CLEANUP_INSTANCE_LINKS)
        // When a render instance or scenario is destroyed unset the scenario. This should happen automatically in the render server
        world.observer<const UniqueRenderInstance, const RenderScenario, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).up(flecs::ChildOf)
            .term_at(2).singleton().filter()
            .each([](const UniqueRenderInstance& instance, const RenderScenario& scenario, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand("instance_set_scenario", instance.id, godot::RID());
        });
#endif

        world.observer<const UniqueRenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceSetMesh"))
            .event(flecs::OnSet)
            .term_at(0).self().second("$Base")
            .term_at(1).src("$Base").filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueRenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(mesh.id != godot::RID(), "Mesh should be valid");

            thread_context.commands.AddCommand("instance_set_base", instance.id, mesh.id);
        });

#if defined(CLEANUP_INSTANCE_LINKS)
        // When a render instance or base is destroyed unset the base. This should happen automatically in the render server
        world.observer<const UniqueRenderInstance, const RenderMesh, RenderingServerContext>(DEBUG_ONLY("RenderInstanceRemoveMesh"))
            .event(flecs::OnRemove)
            .term_at(0).self().second("$Base")
            .term_at(1).src("$Base")
            .term_at(2).singleton().filter()
            .each([](const UniqueRenderInstance& instance, const RenderMesh& mesh, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(mesh.id != godot::RID(), "Mesh should be valid");

            thread_context.commands.AddCommand("instance_set_base", instance.id, godot::RID());
        });
#endif

        // Update the render tree nodes transform based on the current nodes position, rotation, scale and parents transform
        world.system<RenderTreeNode, const Position3DComponent*, const Rotation3DComponent*, const Scale3DComponent*, const RenderTreeNode*>(DEBUG_ONLY("UpdateRenderTreeNodeTransforms"))
            .multi_threaded()
            .term_at(0).self()
            .term_at(1).self()
            .term_at(2).self()
            .term_at(3).self()
            .term_at(4).cascade(flecs::ChildOf)
            .each([](RenderTreeNode& tree_node, const Position3DComponent* position, const Rotation3DComponent* rotation, const Scale3DComponent* scale, const RenderTreeNode* parent_tree_node)
        {
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

            if (parent_tree_node != nullptr)
            {
                transform *= parent_tree_node->transform;

                tree_node.modify_flags.visible = tree_node.visible != parent_tree_node->visible;
                tree_node.visible = tree_node.visible && parent_tree_node->visible;
            }

            tree_node.modify_flags.transform = transform != tree_node.transform;
            tree_node.transform = transform;
        });

        // Update the render instances transform based on the tree node transform given the entity is a tree node
        world.system<UniqueRenderInstance, const RenderTreeNode, RenderingServerContext>(DEBUG_ONLY("UpdateRenderInstanceTransformSelf"))
            .multi_threaded()
            .term_at(0).self().second(flecs::Any)
            .term_at(1).self()
            .term_at(2).singleton()
            .each([](flecs::entity entity, UniqueRenderInstance& instance, const RenderTreeNode& tree_node, RenderingServerContext& context)
        {
            RenderingServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (tree_node.modify_flags.transform)
            {
                thread_context.commands.AddCommand("instance_set_transform", instance.id, tree_node.transform);
            }
        });

        // Update the render instances transform based on the entities position, rotation, scale and parents transform given the entity is not a tree node
        world.system<UniqueRenderInstance, const Position3DComponent*, const Rotation3DComponent*, const Scale3DComponent*, const RenderTreeNode, RenderingServerContext>(DEBUG_ONLY("UpdateRenderInstanceTransformUp"))
            .multi_threaded()
            .term_at(0).self().second(flecs::Any)
            .term_at(1).self()
            .term_at(2).self()
            .term_at(3).self()
            .term_at(4).up(flecs::ChildOf)
            .term_at(5).singleton()
            .without<const RenderTreeNode>().self()
            .each([](flecs::entity entity, UniqueRenderInstance& instance, const Position3DComponent* position, const Rotation3DComponent* rotation, const Scale3DComponent* scale, const RenderTreeNode& parent_tree_node, RenderingServerContext& context)
        {
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

            transform *= parent_tree_node.transform;

            RenderingServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            thread_context.commands.AddCommand("instance_set_transform", instance.id, transform);
        });
	}
}