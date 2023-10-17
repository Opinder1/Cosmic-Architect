#include "FractalWorld3D.h"

#include "FractalNode3D.h"
#include "FractalCamera3D.h"
#include "FractalRegion3D.h"

namespace voxel_world
{
    FractalWorld3D::FractalWorld3D(sim::UUID owning_simulation) :
        m_owning_simulation(owning_simulation)
    {

    }

    FractalWorld3D::~FractalWorld3D()
    {

    }

    uint8_t FractalWorld3D::GetMaxScale() const
    {
        return m_max_scale;
    }

    const FractalNode3D* FractalWorld3D::GetNodeAtCoord(FractalCoord3D coord) const
    {
        FractalRegion3D* region = GetRegionAtCoord(coord);

        if (region)
        {
            return region->GetNodeAtCoord(coord);
        }

        return nullptr;
    }

    FractalRegion3D* FractalWorld3D::GetRegionAtCoord(FractalCoord3D coord) const
    {
        for (FractalRegion3D* region : m_regions)
        {
            if (region->GetRegion().has_point(coord.GetRelPos()))
            {
                return region;
            }
        }

        return nullptr;
    }

    void FractalWorld3D::OnTick()
    {

    }

    void FractalWorld3D::AddCamera(FractalCamera3D& camera)
    {
        m_cameras.push_back(&camera);
    }

    void FractalWorld3D::RemoveCamera(FractalCamera3D& camera)
    {
        auto it = std::find(m_cameras.begin(), m_cameras.end(), &camera);

        if (it != m_cameras.end())
        {
            m_cameras.erase(it);
        }
    }

    void FractalWorld3D::MergeRegions(FractalRegion3D& target, FractalRegion3D& source)
    {

    }
}