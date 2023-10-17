#pragma once

#include "FractalCoord3D.h"

#include "Simulation/UUID.h"

#include "Util/ByteHash.h"

#include <godot_cpp/templates/local_vector.hpp>

#include <godot_cpp/variant/aabb.hpp>

#include <robin_hood/robin_hood.h>

#include <vector>

namespace voxel_world
{
    struct FractalNode3D;
    class FractalWorld3D;
    class FractalCamera3D;

    class FractalRegion3D
    {
        friend class FractalWorld3D;

    public:
        explicit FractalRegion3D(FractalWorld3D& world);
        ~FractalRegion3D();

        godot::AABB GetRegion() const;

        // Get a node a the specified coordinate. Returns nullptr if that node does not exist
        const FractalNode3D* GetNodeAtCoord(FractalCoord3D coord) const;

    protected:
        // A callback to generate a new node
        virtual FractalNode3D* GenerateNode(FractalCoord3D coord) = 0;

        // A callback whenever a node is updated
        virtual void OnNodeUpdate(FractalNode3D& node) = 0;

        // A callback whenever a node is unloaded
        virtual void OnNodeUnload(FractalNode3D& node) = 0;

        // This should be called by the derived class when it updates a node
        void AddToChanged(FractalNode3D& node);

    private:
        FractalNode3D* GetNodeAtCoord(FractalCoord3D coord);

        // Should be called by this regions thread
        void OnTick();

        // Set the box this region will use
        void SetRegion(const godot::AABB& aabb) const;

        // Called when a camera is added to the world and this region should check if it should use it
        void CameraAddedToWorld(FractalCamera3D& camera);

        // Called when a camera is removed from the world
        void CameraRemovedFromWorld(FractalCamera3D& camera);

        // Load a node at the coordinate if its not already loaded
        void LoadNode(FractalCoord3D coord);

        // Unload the node at the coordinate. 
        void UnloadNode(FractalCoord3D coord, bool update_parent = true);

        // Send all this regions data to another region to then unload this one
        void MergeToOtherRegion(FractalRegion3D& destination);

    private:
        FractalWorld3D& m_world;

        sim::UUID m_simulation;

        godot::AABB m_region;

        std::vector<FractalCamera3D*> m_cameras; // List of chunk cameras this region is focused around

        robin_hood::unordered_flat_map<godot::Vector3i, FractalNode3D*, ByteHash<godot::Vector3i>> m_nodes[k_max_scale]; // Random access map for each scale

        robin_hood::unordered_flat_set<FractalNode3D*> m_nodes_changed; // List of nodes marked as updated. Useful for knowing which nodes to update for renderer
        robin_hood::unordered_flat_set<FractalNode3D*> m_nodes_no_observers; // List of nodes that have no observers observing them
    };
}