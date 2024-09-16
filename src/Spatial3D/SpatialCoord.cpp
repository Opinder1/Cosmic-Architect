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

namespace voxel_game::spatial3d
{
    Coord::Coord()
    {}

    Coord::Coord(int32_t x, int32_t y, int32_t z, uint8_t s) :
        x(x),
        y(y),
        z(z),
        scale(s)
    {}

    Coord::Coord(godot::Vector3i pos, uint8_t scale) :
        x(pos.x),
        y(pos.y),
        z(pos.z),
        scale(scale)
    {}

    Coord Coord::GetParent() const
    {
        if (scale % 2 == 0)
        {
            return Coord((pos + 1) / 2, scale + 1);
        }
        else
        {
            return Coord(pos / 2, scale + 1);
        }
    }

    godot::Vector3i Coord::GetParentRelPos() const
    {
        return VecEql(GetParent().GetBottomLeftChild().pos, pos);
    }

    Coord Coord::GetBottomLeftChild() const
    {
        if (scale % 2 == 0)
        {
            return Coord((pos * 2) + godot::Vector3i(0, 0, 0), scale - 1);
        }
        else
        {
            return Coord((pos * 2) + godot::Vector3i(-1, -1, -1), scale - 1);
        }
    }

    godot::Vector3i Coord::GetTileSize() const
    {
        return godot::Vector3i(1 << scale, 1 << scale, 1 << scale);
    }

    godot::Vector3i Coord::GetRelPos() const
    {
        int32_t offset = four_pow(ceil_div2(scale)) / 3;
        int32_t size = 1 << scale;
        return (pos * size) - offset;
    }

    Coord Coord::operator*(uint64_t val)
    {
        return Coord(pos * val, scale);
    }

    Coord Coord::operator/(uint64_t val)
    {
        return Coord(pos / val, scale);
    }

    Coord Coord::operator+(uint64_t val)
    {
        return Coord(pos + val, scale);
    }

    Coord Coord::operator-(uint64_t val)
    {
        return Coord(pos - val, scale);
    }

    Coord Coord::operator%(uint64_t val)
    {
        return Coord(pos % val, scale);
    }
}