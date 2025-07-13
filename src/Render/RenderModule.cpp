#include "RenderModule.h"
#include "RenderComponents.h"

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
        // Get an initial batch of rids for rendering objects
        AllocatorServer::get_singleton()->RequestRIDs(true);

        // Fill our allocators with that initial batch of rids
        for (ThreadContext& thread_context : simulation.thread_contexts)
        {
            thread_context.allocator.Process();
        }
    }

	void Uninitialize(Simulation& simulation)
    {
        CommandServer* cqserver = CommandServer::get_singleton();
        uint64_t rserver_id = RS::get_singleton()->get_instance_id();

        for (ThreadContext& thread_context : simulation.thread_contexts)
        {
            cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
        }
    }

	void Update(Simulation& simulation)
    {
        CommandServer* cqserver = CommandServer::get_singleton();
        uint64_t rserver_id = RS::get_singleton()->get_instance_id();

        for (ThreadContext& thread_context : simulation.thread_contexts)
        {
            thread_context.allocator.Process();

            cqserver->AddCommands(rserver_id, std::move(thread_context.commands));
        }
    }

    void WorkerUpdate(Simulation& simulation, size_t index)
    {

    }

    bool IsEnabled()
    {
        return godot::DisplayServer::get_singleton()->get_name() != "headless";
    }
}