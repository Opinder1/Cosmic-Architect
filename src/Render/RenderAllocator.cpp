#include "RenderAllocator.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include "Util/Debug.h"

namespace voxel_game::rendering
{
    const size_t k_max_preallocated_instances = 512;

    godot::OptObj<AllocatorServer> AllocatorServer::k_singleton;

    AllocatorServer* AllocatorServer::get_singleton()
    {
        return &k_singleton.value();
    }

    void AllocatorServer::_bind_methods()
    {
        k_singleton.instantiate();
    }

    void AllocatorServer::_cleanup_methods()
    {
        k_singleton.reset();
    }

    AllocatorServer::AllocatorServer()
    {

    }

    AllocatorServer::~AllocatorServer()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        for (godot::RID instance : m_write_instances)
        {
            rserver->free_rid(instance);
        }
    }

    void AllocatorServer::Process(std::vector<godot::RID>& read_instances)
    {
        if (m_mutex.try_lock()) // Don't bother if we are currently generating instances
        {
            if (read_instances.size() < m_write_instances.size())
            {
                read_instances.swap(m_write_instances);
            }

            if (m_write_instances.size() < k_max_preallocated_instances)
            {
                godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &AllocatorServer::AllocateRIDs));
            }

            m_mutex.unlock();
        }
    }

    void AllocatorServer::AllocateRIDs()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        std::lock_guard lock(m_mutex);

        while (m_write_instances.size() < k_max_preallocated_instances)
        {
            m_write_instances.push_back(rserver->instance_create());
        }
    }

    Allocator::Allocator()
    {
        Process();
    }

    Allocator::~Allocator()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        for (godot::RID instance : m_read_instances)
        {
            rserver->free_rid(instance);
        }
    }

    void Allocator::Process()
    {
        AllocatorServer::get_singleton()->Process(m_read_instances);
    }

    godot::RID Allocator::CreateInstance()
    {
        godot::RID rid;

        if (m_read_instances.size() > 0)
        {
            rid = m_read_instances.back();
            m_read_instances.pop_back();
        }
        else
        {
            DEBUG_PRINT_WARN("Enough instances were allocated this frame that we are using the slow path");
            rid = godot::RenderingServer::get_singleton()->instance_create();
        }

        return rid;
    }
}