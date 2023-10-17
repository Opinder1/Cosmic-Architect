#pragma once

#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/templates/hashfuncs.hpp>

namespace voxel_world
{
    constexpr const uint8_t k_max_scale = 16;

    // A coordinate in a fractal world that has a position and a level of detail
    class FractalCoord3D
    {
    public:
        FractalCoord3D(int32_t x, int32_t y, int32_t z, uint8_t s);

        FractalCoord3D(godot::Vector3i pos, int8_t s);

        FractalCoord3D GetParent() const;

        template<class Callable>
        void ForEachChild(Callable&& callable) const
        {
            if (m_s % 2 == 0)
            {
                for (int32_t x = 0; x < 2; x++)
                for (int32_t y = 0; y < 2; y++)
                for (int32_t z = 0; z < 2; z++)
                {
                    callable(FractalCoord3D((2 * m_pos) + godot::Vector3i(x, y, z), m_s - 1));
                }
            }
            else
            {
                for (int32_t x = -1; x < 1; x++)
                for (int32_t y = -1; y < 1; y++)
                for (int32_t z = -1; z < 1; z++)
                {
                    callable(FractalCoord3D((2 * m_pos) + godot::Vector3i(x, y, z), m_s - 1));
                }
            }
        }

        godot::Vector3i GetPos() const;

        uint8_t GetScale() const;

        godot::Vector3i GetParentRelPos() const;

        FractalCoord3D GetBottomLeftChild() const;

        godot::Vector3i GetTileSize() const;

        godot::Vector3i GetRelPos() const;

    private:
        godot::Vector3i m_pos; // Position relative to scale
        uint8_t m_s; // Scale
    };

    struct VectorHash
    {
        size_t operator()(const godot::Vector3i& vec) const noexcept;
    };

    struct CoordHash
    {
        size_t operator()(const FractalCoord3D& coord) const noexcept;
    };
}