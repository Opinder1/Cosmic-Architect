#include "FractalRegion3D.h"

#include "FractalNode3D.h"
#include "FractalCamera3D.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/SimulationBuilder.h"

#include "Util/Debug.h"

namespace voxel_world
{
    FractalRegion3D::FractalRegion3D(FractalWorld3D& world) :
        m_world(world)
    {
        m_simulation = sim::SimulationServer::GetSingleton()->CreateSimulation(std::make_unique<sim::EmptySimulationBuilder>(), sim::SimulationServer::CreateMethod::ThreadAutoStart);
    }

    FractalRegion3D::~FractalRegion3D()
    {

    }

    godot::AABB FractalRegion3D::GetRegion() const
    {
        return m_region;
    }

    const FractalNode3D* FractalRegion3D::GetNodeAtCoord(FractalCoord3D coord) const
    {
        if (coord.GetScale() > k_max_scale)
        {
            DEBUG_PRINT_ERROR("Invalid level of detail for node");
            return nullptr;
        }

        auto& layer = m_nodes[coord.GetScale()];

        auto it = layer.find(coord.GetPos());

        if (it == layer.end())
        {
            return nullptr;
        }

        return it->second;
    }

    FractalNode3D* FractalRegion3D::GetNodeAtCoord(FractalCoord3D coord)
    {
        if (coord.GetScale() > k_max_scale)
        {
            DEBUG_PRINT_ERROR("Invalid level of detail for node");
            return nullptr;
        }

        auto& layer = m_nodes[coord.GetScale()];

        auto it = layer.find(coord.GetPos());

        if (it == layer.end())
        {
            return nullptr;
        }

        return it->second;
    }

    void FractalRegion3D::OnTick()
    {
        Clock::time_point now = Clock::now();

        // Update graphics only for changed nodes
        for (FractalNode3D* node : m_nodes_changed)
        {
            node->last_update_time = now;

            OnNodeUpdate(*node);
        }
        m_nodes_changed.clear();

        // Unload any chunks that have no observers for too long. Do here as other lists may otherwise get dangling pointers
        for (FractalNode3D* node : m_nodes_no_observers)
        {
            if (now - node->unload_time > 5s) // 5 seconds
            {
                UnloadNode(node->coord);
            }
        }

        // Update all camera load areas
        for (FractalCamera3D* camera : m_cameras)
        {
            camera->UpdateObserved();
        }
    }

    void FractalRegion3D::AddToChanged(FractalNode3D& node)
    {
        if (&node.world != &m_world)
        {
            DEBUG_PRINT_ERROR("Tried to add node that is not from our world to changelist");
            return;
        }

        m_nodes_changed.insert(&node);
    }

    void FractalRegion3D::LoadNode(FractalCoord3D coord)
    {
        if (coord.GetScale() > k_max_scale)
        {
            DEBUG_PRINT_ERROR("Invalid level of detail for node");
            return;
        }

        auto& layer = m_nodes[coord.GetScale()];

        auto&& [it, success] = layer.emplace(coord.GetPos(), nullptr); // We should be guarenteed to at least get an item when calling this

        if (!success)
        {
            DEBUG_PRINT_ERROR("The node that was loaded already existed");
            return;
        }

        FractalNode3D* node = it->second = GenerateNode(coord);

        // Create parent if we were just created but don't go above max lod
        if (coord.GetScale() < k_max_scale)
        {
            FractalNode3D* parent = GetNodeAtCoord(coord.GetParent());

            if (parent == nullptr)
            {
                DEBUG_PRINT_ERROR("Parent should exist before child");
                return;
            }

            godot::Vector3i rel_pos = node->coord.GetParentRelPos();

            node->parent = parent;
            parent->children[rel_pos.x][rel_pos.y][rel_pos.z] = node;
        }
    }

    void FractalRegion3D::UnloadNode(FractalCoord3D coord, bool update_parent)
    {
        if (coord.GetScale() > k_max_scale)
        {
            DEBUG_PRINT_ERROR("Invalid level of detail for node");
            return;
        }

        // If we are not the highest level of detail then unload children beforehand
        if (coord.GetScale() > 0)
        {
            coord.ForEachChild([this](FractalCoord3D coord)
            {
                UnloadNode(coord, false);
            });
        }

        auto& layer = m_nodes[coord.GetScale()];

        auto it = layer.find(coord.GetPos());

        if (it == layer.end())
        {
            DEBUG_PRINT_ERROR("The requested node did not exist");
            return;
        }

        FractalNode3D* node = it->second;

        // First run the unload callback
        OnNodeUnload(*node);

        // Remove this node from its parent
        if (update_parent && coord.GetScale() < k_max_scale) // Only do if we are the root unloaded node and not the top level node
        {
            FractalNode3D* parent = node->parent;

            godot::Vector3i rel_pos = node->coord.GetParentRelPos();

            parent->children[rel_pos.x][rel_pos.y][rel_pos.z] = nullptr;
        }

        // Remove this chunk from its neighbours
        for (size_t i = 0; i < 6; i++)
        {
            FractalNode3D* neighbour = node->neighbours[i];

            if (neighbour)
            {
                neighbour->neighbours[5 - i] = nullptr;
            }
        }

        // Remove this chunk from no observers list to avoid dangling pointer
        if (node->num_observers == 0)
        {
            m_nodes_no_observers.erase(node);
        }

        memfree(it->second);

        layer.erase(it);
    }
}