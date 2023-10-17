#pragma once

#include "Simulation/UUID.h"

#include "FractalCoord3D.h"

#include <vector>

namespace voxel_world
{
    struct FractalNode3D;
    class FractalRegion3D;
    class FractalCamera3D;
    class FractalGenerator3D;

    // A octree like world with multiple levels of detail as well as random map access
    class FractalWorld3D
    {
        friend class FractalCamera3D;

    public:
        explicit FractalWorld3D(sim::UUID owning_simulation);
        ~FractalWorld3D();

        uint8_t GetMaxScale() const;

        // Get a node a the specified coordinate. Returns nullptr if that node does not exist
        const FractalNode3D* GetNodeAtCoord(FractalCoord3D coord) const;

        // Get the region that the specified coordinate is in
        FractalRegion3D* GetRegionAtCoord(FractalCoord3D coord) const;

        template<class NodeFunc>
        void ApplyFunctionOnRegion() {}

        void CopyBox() {}

        void PasteBox() {}

    protected:
        virtual FractalRegion3D* GenerateRegion() = 0;

    private:
        // Update that should be called every tick by owner thread
        void OnTick();

        // Called by a camera that wants to add itself to this world
        void AddCamera(FractalCamera3D& camera);

        // Called by a camera that wants to remove itself from this world
        void RemoveCamera(FractalCamera3D& camera);

        // Merge a source region into a target region
        void MergeRegions(FractalRegion3D& target, FractalRegion3D& source);

    private:
        sim::UUID m_owning_simulation;

        uint8_t m_max_scale = k_max_scale; // The max scale this world supports

        std::vector<FractalCamera3D*> m_cameras; // List of chunk cameras

        std::vector<FractalRegion3D*> m_regions; // List of regions
    };
}