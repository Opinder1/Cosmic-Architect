#include "FractalCoord.h"
namespace
{
    godot::Vector3i VecEql(godot::Vector3i l, godot::Vector3i r)
    {
        return godot::Vector3i(l.x == r.x, l.y == r.y, l.z == r.z);
    }

    godot::Vector3i operator+(godot::Vector3i l, int32_t r)
    {
        return godot::Vector3i(l.x + r, l.y + r, l.z + r);
    }

    godot::Vector3i operator-(godot::Vector3i l, int32_t r)
    {
        return godot::Vector3i(l.x - r, l.y - r, l.z - r);
    }

    int32_t four_pow(int32_t i)
    {
        return (1 << (i * 2));
    }

    int32_t ceil_div2(int32_t i)
    {
        return (i + 1) / 2;
    }
}

namespace voxel_game
{
    FractalCoord3D::FractalCoord3D()
    {}

    FractalCoord3D::FractalCoord3D(int32_t x, int32_t y, int32_t z, uint8_t s) :
        m_pos(x, y, z),
        m_scale(s)
    {}

    FractalCoord3D::FractalCoord3D(godot::Vector3i pos, uint8_t scale) :
        m_pos(pos),
        m_scale(scale)
    {}

    FractalCoord3D FractalCoord3D::GetParent() const
    {
        if (m_scale % 2 == 0)
        {
            return FractalCoord3D((m_pos + 1) / 2, m_scale + 1);
        }
        else
        {
            return FractalCoord3D(m_pos / 2, m_scale + 1);
        }
    }

    godot::Vector3i FractalCoord3D::GetPos() const
    {
        return m_pos;
    }

    uint8_t FractalCoord3D::GetScale() const
    {
        return m_scale;
    }

    godot::Vector3i FractalCoord3D::GetParentRelPos() const
    {
        return VecEql(GetParent().GetBottomLeftChild().m_pos, m_pos);
    }

    FractalCoord3D FractalCoord3D::GetBottomLeftChild() const
    {
        if (m_scale % 2 == 0)
        {
            return FractalCoord3D((m_pos * 2) + godot::Vector3i(0, 0, 0), m_scale - 1);
        }
        else
        {
            return FractalCoord3D((m_pos * 2) + godot::Vector3i(-1, -1, -1), m_scale - 1);
        }
    }

    godot::Vector3i FractalCoord3D::GetTileSize() const
    {
        return godot::Vector3i(1 << m_scale, 1 << m_scale, 1 << m_scale);
    }

    godot::Vector3i FractalCoord3D::GetRelPos() const
    {
        int32_t offset = four_pow(ceil_div2(m_scale)) / 3;
        int32_t size = 1 << m_scale;
        return (m_pos * size) - offset;
    }
}