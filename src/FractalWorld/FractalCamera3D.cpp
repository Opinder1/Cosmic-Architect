#include "FractalCamera3D.h"

#include "FractalWorld3D.h"
#include "FractalNode3D.h"

#include <godot_cpp/core/error_macros.hpp>

namespace voxel_world
{
    FractalCamera3D::FractalCamera3D(FractalWorld3D& world, FractalCoord3D pos) :
        m_world(world),
        m_pos(pos)
    {
        m_world.AddCamera(*this);
    }

    FractalCamera3D::~FractalCamera3D()
    {
        m_world.RemoveCamera(*this);
    }

    void FractalCamera3D::AddToObserved(FractalNode3D& node)
    {
        ERR_FAIL_COND(&node.world != &m_world);

        m_observed.push_back(&node);

        node.num_observers++;
    }

    void FractalCamera3D::RemoveFromObserved(FractalNode3D& node)
    {
        ERR_FAIL_COND(&node.world != &m_world);

        auto it = std::find(m_observed.begin(), m_observed.end(), &node);

        if (it != m_observed.end())
        {
            node.num_observers--;

            m_observed.erase(it);
        }
    }
}