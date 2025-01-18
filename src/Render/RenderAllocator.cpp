#include "RenderAllocator.h"

#include <godot_cpp/classes/rendering_server.hpp>

#include "Util/Debug.h"

namespace voxel_game::rendering
{
    const size_t k_max_preallocated[k_num_alloc_types] =
    {
        0, // texture_2d
        0, // texture_3d
        0, // shader
        0, // material
        128, // mesh
        0, // multimesh
        0, // skeleton
        0, // directional_light
        0, // omni_light
        0, // spot_light
        0, // reflection_probe
        0, // decal
        0, // voxel_gi
        0, // lightmap
        0, // particles
        0, // particles_collision
        0, // fog_volume
        0, // visibility_notifier
        0, // occluder
        0, // camera
        0, // viewport
        0, // sky
        0, // compositor_effect
        0, // compositor
        0, // environment
        0, // camera_attributes
        0, // scenario
        512, // instance
        0, // canvas
        0, // canvas_texture
        0, // canvas_item
        0, // canvas_light
        0, // canvas_light_occluder
        0, // canvas_occluder_polygon
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

    AllocatorServer::AllocatorServer() {}

    AllocatorServer::~AllocatorServer()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        for (TypeData& allocatable : m_types)
        {
            for (godot::RID rid : allocatable.rids)
            {
                rserver->free_rid(rid);
            }
        }
    }

    void AllocatorServer::RequestRIDs(AllocateType type, std::vector<godot::RID>& rids_out)
    {
        if (m_mutex.try_lock()) // Don't bother if we are currently generating instances
        {
            bool allocate_more = false;

            TypeData& type_data = m_types[to_underlying(type)];

            if (rids_out.size() < type_data.rids.size())
            {
                rids_out.swap(type_data.rids);
                allocate_more = true;
            }

            m_mutex.unlock();

            if (allocate_more)
            {
                AllocateRIDs();
            }
        }
    }

    void AllocatorServer::AllocateRIDs()
    {
        godot::RenderingServer::get_singleton()->call_on_render_thread(callable_mp(this, &AllocatorServer::AllocateRIDsInternal));
    }

    void AllocatorServer::AllocateRIDsInternal()
    {
        godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();

        DEBUG_ASSERT(rserver->is_on_render_thread(), "This should be called when on the render thread as its entire existence is to be performant");

        std::lock_guard lock(m_mutex);

        for (size_t i = 0; i < k_num_alloc_types; i++)
        {
            RIDGenerator generator = k_rid_generators[i];

            while (m_types[i].rids.size() < k_max_preallocated[i])
            {
                m_types[i].rids.push_back((rserver->*generator)());
            }
        }
    }

    Allocator::Allocator(AllocateType type) :
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

            godot::RenderingServer* rserver = godot::RenderingServer::get_singleton();
            RIDGenerator generator = k_rid_generators[to_underlying(m_type)];

            (rserver->*generator)();
        }

        return rid;
    }
}