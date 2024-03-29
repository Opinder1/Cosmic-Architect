#include "SpatialCoord.h"

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
    SpatialCoord3D::SpatialCoord3D()
    {}

    SpatialCoord3D::SpatialCoord3D(int32_t x, int32_t y, int32_t z, uint32_t s) :
        x(x),
        y(y),
        z(z),
        scale(s)
    {}

    SpatialCoord3D::SpatialCoord3D(godot::Vector3i pos, uint32_t scale) :
        x(pos.x),
        y(pos.y),
        z(pos.z),
        scale(scale)
    {}

    SpatialCoord3D SpatialCoord3D::GetParent() const
    {
        if (scale % 2 == 0)
        {
            return SpatialCoord3D((pos + 1) / 2, scale + 1);
        }
        else
        {
            return SpatialCoord3D(pos / 2, scale + 1);
        }
    }

    godot::Vector3i SpatialCoord3D::GetParentRelPos() const
    {
        return VecEql(GetParent().GetBottomLeftChild().pos, pos);
    }

    SpatialCoord3D SpatialCoord3D::GetBottomLeftChild() const
    {
        if (scale % 2 == 0)
        {
            return SpatialCoord3D((pos * 2) + godot::Vector3i(0, 0, 0), scale - 1);
        }
        else
        {
            return SpatialCoord3D((pos * 2) + godot::Vector3i(-1, -1, -1), scale - 1);
        }
    }

    godot::Vector3i SpatialCoord3D::GetTileSize() const
    {
        return godot::Vector3i(1 << scale, 1 << scale, 1 << scale);
    }

    godot::Vector3i SpatialCoord3D::GetRelPos() const
    {
        int32_t offset = four_pow(ceil_div2(scale)) / 3;
        int32_t size = 1 << scale;
        return (pos * size) - offset;
    }

    SpatialCoord3D SpatialCoord3D::operator*(uint64_t val)
    {
        return SpatialCoord3D(pos * val, scale);
    }

    SpatialCoord3D SpatialCoord3D::operator/(uint64_t val)
    {
        return SpatialCoord3D(pos / val, scale);
    }

    SpatialCoord3D SpatialCoord3D::operator+(uint64_t val)
    {
        return SpatialCoord3D(pos + val, scale);
    }

    SpatialCoord3D SpatialCoord3D::operator-(uint64_t val)
    {
        return SpatialCoord3D(pos - val, scale);
    }

    SpatialCoord3D SpatialCoord3D::operator%(uint64_t val)
    {
        return SpatialCoord3D(pos % val, scale);
    }
}