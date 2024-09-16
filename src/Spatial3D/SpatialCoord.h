#pragma once

#include <godot_cpp/variant/vector3i.hpp>

namespace voxel_game::spatial3d
{
    // A coordinate in a Spatial world that has a position and a level of detail
    struct Coord
    {
        Coord();
        Coord(int32_t x, int32_t y, int32_t z, uint8_t scale);
        Coord(godot::Vector3i pos, uint8_t scale);

        Coord GetParent() const;

        godot::Vector3i GetParentRelPos() const;

        Coord GetBottomLeftChild() const;

        godot::Vector3i GetTileSize() const;

        godot::Vector3i GetRelPos() const;

        Coord operator*(uint64_t val);
        Coord operator/(uint64_t val);
        Coord operator+(uint64_t val);
        Coord operator-(uint64_t val);
        Coord operator%(uint64_t val);

        union
        {
            godot::Vector3i pos{};
            struct
            {
                int32_t x; // Position relative to scale
                int32_t y;
                int32_t z;
            };
        };
        uint8_t scale = 0;
    };

    template<class Callable>
    void ForEachChild(const Coord& coord, Callable&& callable)
    {
        if (m_s % 2 == 0)
        {
            for (int32_t x = 0; x < 2; x++)
            for (int32_t y = 0; y < 2; y++)
            for (int32_t z = 0; z < 2; z++)
            {
                callable(Coord((2 * coord.pos) + godot::Vector3i(x, y, z), coord.scale - 1));
            }
        }
        else
        {
            for (int32_t x = -1; x < 1; x++)
            for (int32_t y = -1; y < 1; y++)
            for (int32_t z = -1; z < 1; z++)
            {
                callable(Coord((2 * coord.pos) + godot::Vector3i(x, y, z), coord.scale - 1));
            }
        }
    }
}