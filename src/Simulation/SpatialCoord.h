#pragma once

#include <godot_cpp/variant/vector3i.hpp>

namespace voxel_game
{
    // A coordinate in a Spatial world that has a position and a level of detail
    class SpatialCoord3D
    {
    public:
        SpatialCoord3D();
        SpatialCoord3D(int32_t x, int32_t y, int32_t z, uint8_t scale);
        SpatialCoord3D(godot::Vector3i pos, uint8_t scale);

        SpatialCoord3D GetParent() const;

        godot::Vector3i GetPos() const;

        uint8_t GetScale() const;

        godot::Vector3i GetParentRelPos() const;

        SpatialCoord3D GetBottomLeftChild() const;

        godot::Vector3i GetTileSize() const;

        godot::Vector3i GetRelPos() const;

    private:
        godot::Vector3i m_pos; // Position relative to scale
        uint8_t m_scale = 0;
    };

    template<class Callable>
    void ForEachChild(const SpatialCoord3D& coord, Callable&& callable)
    {
        if (m_s % 2 == 0)
        {
            for (int32_t x = 0; x < 2; x++)
            for (int32_t y = 0; y < 2; y++)
            for (int32_t z = 0; z < 2; z++)
            {
                callable(SpatialCoord3D((2 * coord.GetPos()) + godot::Vector3i(x, y, z), coord.GetScale() - 1));
            }
        }
        else
        {
            for (int32_t x = -1; x < 1; x++)
            for (int32_t y = -1; y < 1; y++)
            for (int32_t z = -1; z < 1; z++)
            {
                callable(SpatialCoord3D((2 * coord.GetPos()) + godot::Vector3i(x, y, z), coord.GetScale() - 1));
            }
        }
    }
}