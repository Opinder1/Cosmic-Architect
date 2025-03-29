#include "RenderModule.h"
#include "RenderComponents.h"
#include "RenderContext.h"

#include "Physics3D/PhysicsComponents.h"

#include "Simulation/SimulationComponents.h"

#include "Commands/CommandServer.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <easy/profiler.h>

namespace voxel_game::rendering
{
    /*
    const godot::Transform3D k_invisible_transform{ godot::Basis(), godot::Vector3(FLT_MAX, FLT_MAX, FLT_MAX) };

    void InitTransform(flecs::world& world)
    {
        // Update the render tree nodes transform based on the current nodes position, rotation, scale and parents transform
        world.system<CTransform, const physics3d::CPosition*, const physics3d::CRotation*, const physics3d::CScale*, const CTransform*>(DEBUG_ONLY("UpdateTreeNodeTransforms"))
            .multi_threaded()
            .term_at(4).cascade(flecs::ChildOf)
            .each([](CTransform& transform, const physics3d::CPosition* position, const physics3d::CRotation* rotation, const physics3d::CScale* scale, const CTransform* parent_transform)
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
    }

    void InitInstance(flecs::world& world)
    {
        world.observer<CInstance>(DEBUG_ONLY("AddInstance"))
            .event(flecs::OnAdd)
            .yield_existing()
            .term_at(0).second(flecs::Any)
            .each([](flecs::iter& it, size_t i, CInstance& instance)
        {
            if (instance.id == godot::RID())
            {
                instance.id = AllocRID(RIDType::Instance);

                flecs::entity entity = it.entity(i);

                // Mark the Instance pair as modified
                entity.modified(it.pair(0));
            }
        });

        world.observer<const CInstance>(DEBUG_ONLY("RemoveInstance"))
            .event(flecs::OnRemove)
            .term_at(0).second(flecs::Any)
            .each([](const CInstance& instance)
        {
            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            AddCommand<&RS::RenderingServer::free_rid>(instance.id);
        });

        // Update the render instances transform based on the entities position, rotation, scale and parents transform given the entity is not a tree node
        world.system<CInstance, const CTransform>(DEBUG_ONLY("UpdateInstanceTransform"))
            .multi_threaded()
            .term_at(0).second(flecs::Any)
            .term_at(1).self().up(flecs::ChildOf)
            .each([](CInstance& instance, const CTransform& transform)
        {
            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");

            if (transform.modified)
            {
                AddCommand<&RS::instance_set_transform>(instance.id, transform.transform);
            }
        });

        world.observer<const CInstance, const CScenario>(DEBUG_ONLY("InstanceSetScenario"))
            .event(flecs::OnSet)
            .yield_existing()
            .term_at(0).second(flecs::Any)
            .term_at(1).up(flecs::ChildOf)
            .each([](const CInstance& instance, const CScenario& scenario)
        {
            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            AddCommand<&RS::instance_set_transform>(instance.id, k_invisible_transform); // Fake invisibility until we set transform
            AddCommand<&RS::instance_set_scenario>(instance.id, scenario.id);
        });

        world.observer<const CInstance, const CBase>(DEBUG_ONLY("InstanceSetBase"))
            .event(flecs::OnSet)
            .yield_existing()
            .term_at(0).second("$Base")
            .term_at(1).src("$Base")
            .each([](const CInstance& instance, const CBase& base)
        {
            DEBUG_ASSERT(instance.id != godot::RID(), "Instance should be valid");
            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            AddCommand<&RS::instance_set_base>(instance.id, base.id);
        });
    }

    void InitScenario(flecs::world& world)
    {
        world.observer<CScenario>(DEBUG_ONLY("AddScenario"))
            .event(flecs::OnAdd)
            .yield_existing()
            .each([](flecs::entity entity, CScenario& scenario)
        {
            if (scenario.id == godot::RID())
            {
                scenario.id = AllocRID(RIDType::Instance);

                entity.modified<CScenario>();
            }
        });

        world.observer<const CScenario>(DEBUG_ONLY("RemoveScenario"))
            .event(flecs::OnRemove)
            .each([](const CScenario& scenario)
        {
            DEBUG_ASSERT(scenario.id != godot::RID(), "Scenario should be valid");

            AddCommand<&RS::RenderingServer::free_rid>(scenario.id);
        });
    }

    void InitPlaceholderCube(flecs::world& world)
    {
        world.observer<const CPlaceholderCube, CBase>(DEBUG_ONLY("AddPlaceholderCube"))
            .event(flecs::OnAdd)
            .yield_existing()
            .each([](flecs::entity entity, const CPlaceholderCube& mesh, CBase& base)
        {
            EASY_BLOCK("AddPlaceholderCube");

            if (base.id == godot::RID())
            {
                base.id = godot::RenderingServer::get_singleton()->get_test_cube();

                entity.modified<CBase>();
            }
        });
    }

    void InitMesh(flecs::world& world)
    {
        world.observer<const CMesh, CBase>(DEBUG_ONLY("AddMesh"))
            .event(flecs::OnAdd)
            .yield_existing()
            .each([](flecs::entity entity, const CMesh& mesh, CBase& base)
        {
            EASY_BLOCK("AddMesh");

            if (base.id == godot::RID())
            {
                base.id = godot::RenderingServer::get_singleton()->mesh_create();

                entity.modified<CBase>();
            }
        });
    }

    void InitBase(flecs::world& world)
    {
        world.observer<const CBase>(DEBUG_ONLY("RemoveBase"))
            .event(flecs::OnRemove)
            .term_at(0)
            .each([](const CBase& base)
        {
            DEBUG_ASSERT(base.id != godot::RID(), "Base should be valid");

            AddCommand<&RS::free_rid>(base.id);
        });

        InitPlaceholderCube(world);
        InitMesh(world);
    }

	Module::Module(flecs::world& world)
	{
		world.module<Module>();

		world.import<Components>();
        world.import<physics3d::Components>();

        // Make sure we initially get some rids
        AllocatorServer::get_singleton()->RequestRIDs(true);

        world.add<CContext>();

        // Flush each threads render commands to the command queue server which will run them on the rendering server thread
        world.system<CContext>(DEBUG_ONLY("ContextFlushRenderCommands"))
            .immediate()
            .each([](CContext& context)
        {
            CommandServer* cqserver = CommandServer::get_singleton();
            uint64_t rserver_id = RS::get_singleton()->get_instance_id();

            for (ThreadContext& thread_context : context.threads)
            {
                cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
            }
        });

        world.observer<CContext>(DEBUG_ONLY("ContextCleanupCommands"))
            .event(flecs::OnRemove)
            .each([](CContext& context)
        {
            CommandServer* cqserver = CommandServer::get_singleton();
            uint64_t rserver_id = RS::get_singleton()->get_instance_id();

            for (ThreadContext& thread_context : context.threads)
            {
                cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
            }
        });

        world.system<CContext>(DEBUG_ONLY("ContextPreallocateObjects"))
            .immediate()
            .each([](CContext& context)
        {
            for (ThreadContext& thread_context : context.threads)
            {
                thread_context.allocator.Process();
            }
        });

        world.observer<CContext>(DEBUG_ONLY("ContextSetMainThreadContext"))
            .yield_existing()
            .event(flecs::OnAdd)
            .each([](CContext& context)
        {
            SetContext(context.threads[0]);
        });

        world.system<const sim::CThreadWorker, CContext>(DEBUG_ONLY("ContextSetThreadContexts"))
            .multi_threaded()
            .term_at(1).singleton()
            .each([](flecs::iter& it, size_t i, const sim::CThreadWorker& worker, CContext& context)
        {
            SetContext(context.threads[it.world().get_stage_id()]);
        });

        InitTransform(world);
        InitScenario(world);
        InitInstance(world);
        InitBase(world);
	}
    */

    bool IsEnabled()
    {
        return godot::DisplayServer::get_singleton()->get_name() != "headless";
    }
}