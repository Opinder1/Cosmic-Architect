#pragma once

#include "FractalCoord3D.h"

#include <vector>

namespace voxel_world
{
    struct FractalNode3D;
    class FractalWorld3D;
    class FractalRegion3D;

    // A camera that loads chunks in a fractal world around itself
    class FractalCamera3D
    {
    public:
        FractalCamera3D(FractalWorld3D& world, FractalCoord3D coord);

        ~FractalCamera3D();

        void SetPosition(const FractalCoord3D& coord);
        const FractalCoord3D& GetPosition() const;

        void SetDistancePerLOD(size_t distance);
        size_t GetDistancePerLOD() const;

        void SetMinimumLOD(size_t value);
        size_t GetMinimumLOD() const;

        void SetMaximumLOD(size_t value);
        size_t GetMaximumLOD() const;

        void SetUpdateFrequency(size_t value);
        size_t GetUpdateFrequency() const;

        // Called by the world when it loads new chunks for the camera
        virtual void UpdateObserved() const = 0;

    protected:
        void AddToObserved(FractalNode3D& node);

        void RemoveFromObserved(FractalNode3D& node);

    private:
        FractalWorld3D& m_world; // World that this camera is loading chunks in
        FractalRegion3D* m_region; // Region that this camera is in
        FractalCoord3D m_pos; // Position relative to the world

        size_t m_dist_per_lod; // The number of nodes there are until the next lod starts
        size_t m_min_lod; // The minimum lod this camera can see
        size_t m_max_lod; // The maximum lod this camera can see
        size_t m_update_frequency; // The frequency

        std::vector<FractalNode3D*> m_observed; // Nodes that are being observed by this camera
    };
}