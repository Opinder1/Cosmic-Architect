#include "RenderModule.h"
#include "RenderComponents.h"
#include "TreeComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/CommandQueue.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include <flecs/flecs.h>

#include <easy/profiler.h>

namespace voxel_game::rendering
{
    const godot::Transform3D k_invisible_transform{ godot::Basis(), godot::Vector3(FLT_MAX, FLT_MAX, FLT_MAX) };

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
        world.import<TreeComponents>();
        world.import<physics3d::Components>();

        world.add<ServerContext>();

        AllocatorServer::get_singleton()->AllocateRIDs();

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<ServerContext>(DEBUG_ONLY("FrameFlushCommands"))
            .immediate()
            .each([](ServerContext& context)
        {
            CommandQueueServer* cqserver = CommandQueueServer::get_singleton();
            uint64_t rserver_id = godot::RenderingServer::get_singleton()->get_instance_id();

            for (ServerThreadContext& thread_context : context.threads)
            {
                cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
            }

            // Do main thread commands last as they may include freeing of rids
            cqserver->AddCommands(rserver_id, std::move(context.main_thread.commands));
        });

        world.observer<ServerContext>()
            .event(flecs::OnRemove)
            .each([](ServerContext& context)
        {
            CommandQueueServer* cqserver = CommandQueueServer::get_singleton();
            uint64_t rserver_id = godot::RenderingServer::get_singleton()->get_instance_id();

            for (ServerThreadContext& thread_context : context.threads)
            {
                cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
            }

            // Do main thread commands last as they may include freeing of rids
            cqserver->AddCommands(rserver_id, std::move(context.main_thread.commands));
        });

        world.system<ServerContext>(DEBUG_ONLY("PreallocateObjects"))
            .immediate()
            .each([](ServerContext& context)
        {
            context.instance_allocator.Process();
        });

        InitTree(world);
        InitScenario(world);
        InitUniqueInstance(world);
        InitBase(world);
	}

    void Module::InitTree(flecs::world& world)
    {
        // Update the render tree nodes transform based on the current nodes position, rotation, scale and parents transform
        world.system<TreeNode, const physics3d::Position*, const physics3d::Rotation*, const physics3d::Scale*, const TreeNode*>(DEBUG_ONLY("UpdateTreeNodeTransforms"))
            .multi_threaded()
            .term_at(0).self()
            .term_at(1).self()
            .term_at(2).self()
            .term_at(3).self()
            .term_at(4).cascade(flecs::ChildOf)
            .each([](TreeNode& tree_node, const physics3d::Position* position, const physics3d::Rotation* rotation, const physics3d::Scale* scale, const TreeNode* parent_tree_node)
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

                tree_node.modify_flags[ModifyFlags::Visible] = tree_node.visible != parent_tree_node->visible;
                tree_node.visible = tree_node.visible && parent_tree_node->visible;
            }

            tree_node.modify_flags[ModifyFlags::Transform] = transform != tree_node.transform;
            tree_node.transform = transform;
        });

        InitTreeTransform(world);
    }

    void Module::InitTreeTransform(flecs::world& world)
    {
        // Update the render instances transform based on the tree node transform given the entity is a tree node
        world.system<UniqueInstance, const TreeNode, ServerContext>(DEBUG_ONLY("UpdateInstanceTransformSelf"))
            .multi_threaded()
            .term_at(0).self().second(flecs::Any)
            .term_at(1).self()
            .term_at(2).singleton()
            .each([](flecs::entity entity, UniqueInstance& instance, const TreeNode& tree_node, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (tree_node.modify_flags[ModifyFlags::Transform])
            {
                thread_context.commands.AddCommand("instance_set_transform", instance.id, tree_node.transform);
            }
        });

        // Update the render instances transform based on the entities position, rotation, scale and parents transform given the entity is not a tree node
        world.system<UniqueInstance, const physics3d::Position*, const physics3d::Rotation*, const physics3d::Scale*, const TreeNode, ServerContext>(DEBUG_ONLY("UpdateInstanceTransformUp"))
            .multi_threaded()
            .term_at(0).self().second(flecs::Any)
            .term_at(1).self()
            .term_at(2).self()
            .term_at(3).self()
            .term_at(4).up(flecs::ChildOf)
            .term_at(5).singleton()
            .without<const TreeNode>().self()
            .each([](flecs::entity entity, UniqueInstance& instance, const physics3d::Position* position, const physics3d::Rotation* rotation, const physics3d::Scale* scale, const TreeNode& parent_tree_node, ServerContext& context)
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

            ServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            thread_context.commands.AddCommand("instance_set_transform", instance.id, transform);
        });
    }
    
    void Module::InitScenario(flecs::world& world)
    {
        world.observer<Scenario, const ServerContext>(DEBUG_ONLY("AddScenario"))
            .event(flecs::OnAdd)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .with<const OwnedScenario>().self()
            .each([](flecs::entity entity, Scenario& scenario, const ServerContext& context)
        {
            EASY_BLOCK("AddScenario");

            scenario.id = godot::RenderingServer::get_singleton()->scenario_create();

            entity.modified<Scenario>();
        });

        world.observer<const Scenario, ServerContext>(DEBUG_ONLY("RemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .with<const OwnedScenario>().self()
            .each([](const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand("free_rid", scenario.id);
        });

        world.observer<const UniqueInstance, const Scenario, ServerContext>(DEBUG_ONLY("InstanceSetScenario"))
            .event(flecs::OnSet)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).up(flecs::ChildOf).filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand("instance_set_transform", instance.id, k_invisible_transform); // Fake invisibility until we set transform
            thread_context.commands.AddCommand("instance_set_scenario", instance.id, scenario.id);
        });

#if defined(DEBUG_ENABLED)
        // When a render instance or scenario is destroyed unset the scenario. This should happen automatically in the render server
        world.observer<const UniqueInstance, const Scenario, ServerContext>(DEBUG_ONLY("InstanceRemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).up(flecs::ChildOf)
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (scenario.id != godot::RID())
            {
                thread_context.commands.AddCommand("instance_set_scenario", instance.id, godot::RID());
            }
        });
#endif
    }

    void Module::InitUniqueInstance(flecs::world& world)
    {
        world.observer<UniqueInstance, ServerContext>(DEBUG_ONLY("AddUniqueInstance"))
            .event(flecs::OnAdd)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](flecs::iter& it, size_t i, UniqueInstance& instance, ServerContext& context)
        {
            EASY_BLOCK("AddUniqueInstance");

            instance.id = context.instance_allocator.RequestRID();

            it.entity(i).modified(it.pair(0));
        });

        world.observer<const UniqueInstance, ServerContext>(DEBUG_ONLY("RemoveUniqueInstance"))
            .event(flecs::OnRemove)
            .term_at(0).self().second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](const UniqueInstance& instance, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            thread_context.commands.AddCommand("free_rid", instance.id);
        });
    }

    void Module::InitBase(flecs::world& world)
    {
        world.observer<const UniqueInstance, const Base, ServerContext>(DEBUG_ONLY("InstanceSetBase"))
            .event(flecs::OnSet)
            .term_at(0).self().second("$Base")
            .term_at(1).src("$Base").filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            thread_context.commands.AddCommand("instance_set_base", instance.id, base.id);
        });

        world.observer<const Base, ServerContext>(DEBUG_ONLY("RemoveBase"))
            .event(flecs::OnRemove)
            .term_at(0).self()
            .term_at(1).singleton().filter()
            .each([](const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            thread_context.commands.AddCommand("free_rid", base.id);
        });

#if defined(DEBUG_ENABLED)
        // When a render instance or base is destroyed unset the base. This should happen automatically in the render server
        world.observer<const UniqueInstance, const Base, ServerContext>(DEBUG_ONLY("RenderInstanceRemoveBase"))
            .event(flecs::OnRemove)
            .term_at(0).self().second("$Base")
            .term_at(1).src("$Base")
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (base.id != godot::RID())
            {
                thread_context.commands.AddCommand("instance_set_base", instance.id, godot::RID());
            }
        });
#endif

        InitPlaceholderCube(world);
        InitMesh(world);
    }

    void Module::InitPlaceholderCube(flecs::world& world)
    {
        world.observer<const PlaceholderCube, Base, const ServerContext>(DEBUG_ONLY("AddPlaceholderCube"))
            .event(flecs::OnAdd)
            .term_at(0).self()
            .term_at(1).self()
            .term_at(2).singleton().filter()
            .each([](flecs::entity entity, const PlaceholderCube& mesh, Base& base, const ServerContext& context)
        {
            EASY_BLOCK("AddPlaceholderCube");

            base.id = godot::RenderingServer::get_singleton()->get_test_cube();

            entity.modified<Base>();
        });
    }

    void Module::InitMesh(flecs::world& world)
    {
        world.observer<const Mesh, Base, const ServerContext>(DEBUG_ONLY("AddMesh"))
            .event(flecs::OnAdd)
            .term_at(0).self()
            .term_at(1).self()
            .term_at(2).singleton().filter()
            .each([](flecs::entity entity, const Mesh& mesh, Base& base, const ServerContext& context)
        {
            EASY_BLOCK("AddMesh");

            base.id = godot::RenderingServer::get_singleton()->mesh_create();

            entity.modified<Base>();
        });
    }
}