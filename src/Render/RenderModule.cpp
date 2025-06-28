#include "RenderModule.h"
#include "RenderComponents.h"
#include "RenderContext.h"

#include "UniverseSimulation.h"

#include "Physics3D/PhysicsComponents.h"

#include "Commands/CommandServer.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include <easy/profiler.h>

namespace voxel_game::rendering
{
	void Initialize(Simulation& simulation)
    {
        AllocatorServer::get_singleton()->RequestRIDs(true);

        simulation.rendering_contexts.resize(simulation.processor_count);

        SetContext(simulation.rendering_contexts[0]);
    }

	void Uninitialize(Simulation& simulation)
    {
        CommandServer* cqserver = CommandServer::get_singleton();
        uint64_t rserver_id = RS::get_singleton()->get_instance_id();

        for (ThreadContext& thread_context : simulation.rendering_contexts)
        {
            cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
        }
    }

	void Update(Simulation& simulation)
    {
        CommandServer* cqserver = CommandServer::get_singleton();
        uint64_t rserver_id = RS::get_singleton()->get_instance_id();

        for (ThreadContext& thread_context : simulation.rendering_contexts)
        {
            thread_context.allocator.Process();

            cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
        }
    }

    void WorkerUpdate(Simulation& simulation, size_t index)
    {
        SetContext(simulation.rendering_contexts[index]);
    }

    bool IsEnabled()
    {
        return godot::DisplayServer::get_singleton()->get_name() != "headless";
    }
}