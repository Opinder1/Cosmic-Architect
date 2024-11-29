#include "RenderAllocator.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include "Util/Debug.h"

namespace voxel_game::rendering
{
    const size_t k_max_preallocated_meshes = 128;
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

        for (godot::RID mesh : m_meshes)
        {
            rserver->free_rid(mesh);
        }

        for (godot::RID instance : m_instances)
        {
            rserver->free_rid(instance);
        }
    }

    void AllocatorServer::RequestRIDs(AllocatorType type, std::vector<godot::RID>& rids_out)
    {
        if (m_mutex.try_lock()) // Don't bother if we are currently generating instances
        {
            bool allocate_more = false;

            switch (type)
            {
            case AllocatorType::Mesh:
                if (rids_out.size() < m_meshes.size())
                {
                    rids_out.swap(m_meshes);
                    allocate_more = true;
                }
                break;

            case AllocatorType::Instance:
                if (rids_out.size() < m_instances.size())
                {
                    rids_out.swap(m_instances);
                    allocate_more = true;
                }
                break;

            default:
                DEBUG_PRINT_ERROR("Preallocation for requested type is not implemented");
                break;
            }

            m_mutex.unlock();

            if (allocate_more)
            {
                godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &AllocatorServer::AllocateRIDs));
            }
        }
    }

    void AllocatorServer::AllocateRIDs()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        std::lock_guard lock(m_mutex);

        while (m_meshes.size() < k_max_preallocated_meshes)
        {
            m_meshes.push_back(rserver->mesh_create());
        }

        while (m_instances.size() < k_max_preallocated_instances)
        {
            m_instances.push_back(rserver->instance_create());
        }
    }

    Allocator::Allocator(AllocatorType type) :
        m_type(type)
    {
        Process();
    }

    Allocator::~Allocator()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        for (godot::RID instance : m_rids)
        {
            rserver->free_rid(instance);
        }
    }

    void Allocator::Process()
    {
        AllocatorServer::get_singleton()->RequestRIDs(m_type, m_rids);
    }

    godot::RID Allocator::RequestRID()
    {
        godot::RID rid;

        if (m_rids.size() > 0)
        {
            rid = m_rids.back();
            m_rids.pop_back();
        }
        else
        {
            DEBUG_PRINT_WARN("Enough rids were allocated this frame that we are using the slow path");

            switch (m_type)
            {
            case AllocatorType::Mesh:
                rid = godot::RenderingServer::get_singleton()->mesh_create();
                break;

            case AllocatorType::Instance:
                rid = godot::RenderingServer::get_singleton()->instance_create();
                break;

            default:
                DEBUG_PRINT_ERROR("Preallocation for requested type is not implemented");
                break;
            }
        }

        return rid;
    }
}