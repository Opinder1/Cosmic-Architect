#include "RenderAllocator.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include "Util/Debug.h"

namespace
{
    bool CheckRenderingServerThread(godot::RenderingServer* rserver)
    {
        if (rserver->has_feature(godot::RenderingServer::FEATURE_MULTITHREADED))
        {
            return rserver->is_on_render_thread();
        }
        else
        {
            return true;
        }
    }
}

namespace voxel_game::rendering
{
    const size_t k_max_preallocated[k_num_alloc_types] =
    {
        0,      // texture_2d
        0,      // texture_3d
        0,      // shader
        64,     // material
        128,    // mesh
        0,      // multimesh
        0,      // skeleton
        0,      // directional_light
        0,      // omni_light
        0,      // spot_light
        0,      // reflection_probe
        0,      // decal
        0,      // voxel_gi
        0,      // lightmap
        0,      // particles
        0,      // particles_collision
        0,      // fog_volume
        0,      // visibility_notifier
        0,      // occluder
        0,      // camera
        0,      // viewport
        0,      // sky
        0,      // compositor_effect
        0,      // compositor
        0,      // environment
        0,      // camera_attributes
        0,      // scenario
        512,    // instance
        0,      // canvas
        0,      // canvas_texture
        0,      // canvas_item
        0,      // canvas_light
        0,      // canvas_light_occluder
        0,      // canvas_occluder_polygon
    };

    using RIDGenerator = godot::RID(godot::RenderingServer::*)();

    RIDGenerator k_rid_generators[k_num_alloc_types] =
    {
        &godot::RenderingServer::texture_2d_placeholder_create,
        &godot::RenderingServer::texture_3d_placeholder_create,
        &godot::RenderingServer::shader_create,
        &godot::RenderingServer::material_create,
        &godot::RenderingServer::mesh_create,
        &godot::RenderingServer::multimesh_create,
        &godot::RenderingServer::skeleton_create,
        &godot::RenderingServer::directional_light_create,
        &godot::RenderingServer::omni_light_create,
        &godot::RenderingServer::spot_light_create,
        &godot::RenderingServer::reflection_probe_create,
        &godot::RenderingServer::decal_create,
        &godot::RenderingServer::voxel_gi_create,
        &godot::RenderingServer::lightmap_create,
        &godot::RenderingServer::particles_create,
        &godot::RenderingServer::particles_collision_create,
        &godot::RenderingServer::fog_volume_create,
        &godot::RenderingServer::visibility_notifier_create,
        &godot::RenderingServer::occluder_create,
        &godot::RenderingServer::camera_create,
        &godot::RenderingServer::viewport_create,
        &godot::RenderingServer::sky_create,
        &godot::RenderingServer::compositor_effect_create,
        &godot::RenderingServer::compositor_create,
        &godot::RenderingServer::environment_create,
        &godot::RenderingServer::camera_attributes_create,
        &godot::RenderingServer::scenario_create,
        &godot::RenderingServer::instance_create,
        &godot::RenderingServer::canvas_create,
        &godot::RenderingServer::canvas_texture_create,
        &godot::RenderingServer::canvas_item_create,
        &godot::RenderingServer::canvas_light_create,
        &godot::RenderingServer::canvas_light_occluder_create,
        &godot::RenderingServer::canvas_occluder_polygon_create,
    };

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
    {}

    AllocatorServer::~AllocatorServer()
    {
        godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &AllocatorServer::DeallocateRIDsInternal));
        m_requests++;

        while (m_requests.load(std::memory_order_relaxed))
        {
            std::this_thread::sleep_for(1s);
        }
    }

    void AllocatorServer::RequestRIDs(bool sync)
    {
        if (m_requests.load(std::memory_order_relaxed) == 0) // Only request if no other requests are in progress
        {
            godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &AllocatorServer::AllocateRIDsInternal));
            m_requests++;

            if (sync)
            {
                godot::RenderingServer::get_singleton()->force_sync();
            }
        }
    }

    void AllocatorServer::GetRIDs(Span<uint16_t, k_num_alloc_types> requested, Span<std::vector<godot::RID>, k_num_alloc_types> all_rids_out)
    {
        if (!std::any_of(requested.begin(), requested.end(), [](uint16_t request) { return request > 0; }))
        {
            return; // Avoid locking mutex
        }

        std::unique_lock lock(m_mutex);

        size_t total_given = 0;

        for (size_t i = 0; i < all_rids_out.Size(); i++)
        {
            RIDType type = static_cast<RIDType>(i);
            std::vector<godot::RID>& rids_out = all_rids_out[i];

            const size_t required = requested[i];
            const size_t needed = required - rids_out.size();
            const size_t max_per_request = k_max_preallocated[to_underlying(type)] / 16;

            if (needed == 0) // No rids are needed
            {
                continue;
            }

            TypeData& type_data = m_types[to_underlying(type)];

            const size_t have = type_data.rids.size();

            size_t giving = have < needed ? have : needed;

            giving = std::min(giving, max_per_request);

            if (giving > 0)
            {
                rids_out.insert(rids_out.end(), type_data.rids.end() - giving, type_data.rids.end());
                type_data.rids.erase(type_data.rids.end() - giving, type_data.rids.end());
            }

            total_given += giving;
        }

        lock.unlock();

        if (total_given > 0)
        {
            RequestRIDs(false);
        }
    }

    void AllocatorServer::FreeRIDs(Span<std::vector<godot::RID>, k_num_alloc_types> all_rids_in)
    {
        std::lock_guard lock(m_mutex);

        for (size_t i = 0; i < all_rids_in.Size(); i++)
        {
            RIDType type = static_cast<RIDType>(i);
            std::vector<godot::RID>& rids_in = all_rids_in[i];

            if (rids_in.size() == 0) // No rids to deallocate
            {
                continue;
            }

            TypeData& type_data = m_types[to_underlying(type)];

            type_data.rids.insert(type_data.rids.end(), rids_in.begin(), rids_in.end());

            rids_in.clear();
        }
    }

    bool AllocatorServer::AnyRequired()
    {
        for (size_t i = 0; i < k_num_alloc_types; i++)
        {
            RIDGenerator generator = k_rid_generators[i];

            if (m_types[i].rids.size() < k_max_preallocated[i])
            {
                return true;
            }
        }
        
        return false;
    }

    void AllocatorServer::AllocateRIDsInternal()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        DEBUG_ASSERT(CheckRenderingServerThread(rserver), "This should be called when on the render thread as its entire existence is to be performant");

        if (!AnyRequired())
        {
            return; // Avoid locking the mutex
        }

        std::lock_guard lock(m_mutex);

        for (size_t i = 0; i < k_num_alloc_types; i++)
        {
            RIDGenerator generator = k_rid_generators[i];

            while (m_types[i].rids.size() < k_max_preallocated[i])
            {
                m_types[i].rids.push_back((rserver->*generator)());
            }
        }

        m_requests--;
    }

    void AllocatorServer::DeallocateRIDsInternal()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        DEBUG_ASSERT(CheckRenderingServerThread(rserver), "This should be called when on the render thread as its entire existence is to be performant");

        std::lock_guard lock(m_mutex);

        for (TypeData& allocatable : m_types)
        {
            for (godot::RID rid : allocatable.rids)
            {
                rserver->free_rid(rid);
            }
        }

        m_requests--;
    }

    Allocator::Allocator()
    {
        Process();
    }

    Allocator::~Allocator()
    {
        AllocatorServer::get_singleton()->FreeRIDs(m_rids);
    }

    void Allocator::Process()
    {
        uint16_t requests[k_num_alloc_types];

        for (size_t i = 0; i < k_num_alloc_types; i++)
        {
            requests[i] = k_max_preallocated[i];
        }

        AllocatorServer::get_singleton()->GetRIDs(requests, m_rids);
    }

    godot::RID Allocator::GetRID(RIDType type)
    {
        godot::RID rid;

        std::vector<godot::RID>& rids = m_rids[to_underlying(type)];

        if (rids.size() > 0)
        {
            rid = rids.back();
            rids.pop_back();
        }

        DEBUG_ASSERT(rid.is_valid(), "The rid should be valid");

        return rid;
    }

    godot::RID Allocator::EnsureRID(RIDType type)
    {
        godot::RID rid = GetRID(type);

        if (!rid.is_valid())
        {
            DEBUG_PRINT_WARN("Enough rids were allocated this frame that we are using the slow path");
            DEBUG_CRASH();

            godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();
            RIDGenerator generator = k_rid_generators[to_underlying(type)];

            rid = (rserver->*generator)();
        }

        DEBUG_ASSERT(rid.is_valid(), "The rid should be valid");

        return rid;
    }
}