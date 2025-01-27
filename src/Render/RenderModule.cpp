#include "RenderModule.h"
#include "RenderComponents.h"

#include "Physics3D/PhysicsComponents.h"

#include "Util/CommandServer.h"

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
        world.import<physics3d::Components>();

        world.add<ServerContext>();

        AllocatorServer::get_singleton()->AllocateRIDs();

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<ServerContext>(DEBUG_ONLY("FrameFlushCommands"))
            .immediate()
            .each([](ServerContext& context)
        {
            CommandServer* cqserver = CommandServer::get_singleton();
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
            CommandServer* cqserver = CommandServer::get_singleton();
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

        InitTransform(world);
        InitScenario(world);
        InitUniqueInstance(world);
        InitBase(world);
	}

    void Module::InitTransform(flecs::world& world)
    {
        // Update the render tree nodes transform based on the current nodes position, rotation, scale and parents transform
        world.system<Transform, const physics3d::Position*, const physics3d::Rotation*, const physics3d::Scale*, const Transform*>(DEBUG_ONLY("UpdateTreeNodeTransforms"))
            .multi_threaded()
            .term_at(0)
            .term_at(1)
            .term_at(2)
            .term_at(3)
            .term_at(4).cascade(flecs::ChildOf)
            .each([](Transform& transform, const physics3d::Position* position, const physics3d::Rotation* rotation, const physics3d::Scale* scale, const Transform* parent_transform)
        {
            godot::Transform3D new_transform;

            if (scale != nullptr)
            {
                new_transform.scale(scale->scale);
            }

            if (rotation != nullptr)
            {
                new_transform.rotate(rotation->rotation.get_axis(), rotation->rotation.get_angle());
            }

            if (position != nullptr)
            {
                new_transform.set_origin(position->position);
            }

            if (parent_transform != nullptr)
            {
                new_transform *= parent_transform->transform;
            }

            transform.modified = transform.transform != new_transform;
            transform.transform = new_transform;
        });

        // Update the render instances transform based on the entities position, rotation, scale and parents transform given the entity is not a tree node
        world.system<UniqueInstance, const Transform, ServerContext>(DEBUG_ONLY("UpdateInstanceTransform"))
            .multi_threaded()
            .term_at(0).second(flecs::Any)
            .term_at(1).self().up(flecs::ChildOf)
            .term_at(2).singleton()
            .each([](flecs::entity entity, UniqueInstance& instance, const Transform& transform, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.threads[entity.world().get_stage_id()];

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (transform.modified)
            {
                thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_transform>(instance.id, transform.transform);
            }
        });
    }
    
    void Module::InitScenario(flecs::world& world)
    {
        world.observer<Scenario, const ServerContext>(DEBUG_ONLY("AddScenario"))
            .event(flecs::OnAdd)
            .yield_existing()
            .term_at(0)
            .term_at(1).singleton().filter()
            .with<const OwnedScenario>()
            .each([](flecs::entity entity, Scenario& scenario, const ServerContext& context)
        {
            EASY_BLOCK("AddScenario");

            scenario.id = godot::RenderingServer::get_singleton()->scenario_create();

            entity.modified<Scenario>();
        });

        world.observer<const Scenario, ServerContext>(DEBUG_ONLY("RemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0)
            .term_at(1).singleton().filter()
            .with<const OwnedScenario>()
            .each([](const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand<&godot::RenderingServer::free_rid>(scenario.id);
        });

        world.observer<const UniqueInstance, const Scenario, ServerContext>(DEBUG_ONLY("InstanceSetScenario"))
            .event(flecs::OnSet)
            .yield_existing()
            .term_at(0).second(flecs::Any)
            .term_at(1).up(flecs::ChildOf).filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_transform>(instance.id, k_invisible_transform); // Fake invisibility until we set transform
            thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_scenario>(instance.id, scenario.id);
        });

#if defined(DEBUG_ENABLED)
        // When a render instance or scenario is destroyed unset the scenario. This should happen automatically in the render server
        world.observer<const UniqueInstance, const Scenario, ServerContext>(DEBUG_ONLY("InstanceRemoveScenario"))
            .event(flecs::OnRemove)
            .term_at(0).second(flecs::Any)
            .term_at(1).up(flecs::ChildOf)
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Scenario& scenario, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (scenario.id != godot::RID())
            {
                thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_scenario>(instance.id, godot::RID());
            }
        });
#endif
    }

    void Module::InitUniqueInstance(flecs::world& world)
    {
        world.observer<UniqueInstance, ServerContext>(DEBUG_ONLY("AddUniqueInstance"))
            .event(flecs::OnAdd)
            .yield_existing()
            .term_at(0).second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](flecs::iter& it, size_t i, UniqueInstance& instance, ServerContext& context)
        {
            EASY_BLOCK("AddUniqueInstance");

            instance.id = context.instance_allocator.RequestRID();

            it.entity(i).modified(it.pair(0));
        });

        world.observer<const UniqueInstance, ServerContext>(DEBUG_ONLY("RemoveUniqueInstance"))
            .event(flecs::OnRemove)
            .term_at(0).second(flecs::Any)
            .term_at(1).singleton().filter()
            .each([](const UniqueInstance& instance, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            thread_context.commands.AddCommand<&godot::RenderingServer::free_rid>(instance.id);
        });
    }

    void Module::InitBase(flecs::world& world)
    {
        world.observer<const UniqueInstance, const Base, ServerContext>(DEBUG_ONLY("InstanceSetBase"))
            .event(flecs::OnSet)
            .yield_existing()
            .term_at(0).second("$Base")
            .term_at(1).src("$Base").filter()
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_base>(instance.id, base.id);
        });

        world.observer<const Base, ServerContext>(DEBUG_ONLY("RemoveBase"))
            .event(flecs::OnRemove)
            .term_at(0)
            .term_at(1).singleton().filter()
            .each([](const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            thread_context.commands.AddCommand<&godot::RenderingServer::free_rid>(base.id);
        });

#if defined(DEBUG_ENABLED)
        // When a render instance or base is destroyed unset the base. This should happen automatically in the render server
        world.observer<const UniqueInstance, const Base, ServerContext>(DEBUG_ONLY("RenderInstanceRemoveBase"))
            .event(flecs::OnRemove)
            .term_at(0).second("$Base")
            .term_at(1).src("$Base")
            .term_at(2).singleton().filter()
            .each([](const UniqueInstance& instance, const Base& base, ServerContext& context)
        {
            ServerThreadContext& thread_context = context.main_thread;

            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (base.id != godot::RID())
            {
                thread_context.commands.AddCommand<&godot::RenderingServer::instance_set_base>(instance.id, godot::RID());
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
            .yield_existing()
            .term_at(0)
            .term_at(1)
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
            .yield_existing()
            .term_at(0)
            .term_at(1)
            .term_at(2).singleton().filter()
            .each([](flecs::entity entity, const Mesh& mesh, Base& base, const ServerContext& context)
        {
            EASY_BLOCK("AddMesh");

            base.id = godot::RenderingServer::get_singleton()->mesh_create();

            entity.modified<Base>();
        });
    }
}