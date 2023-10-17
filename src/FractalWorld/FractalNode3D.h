#pragma once

#include "Simulation/Time.h"

#include "FractalCoord3D.h"

namespace voxel_world
{
    class FractalWorld3D;
    class FractalRegion3D;

    // A single node in a fractal world
    struct FractalNode3D
    {
        FractalNode3D(FractalWorld3D& world, FractalCoord3D coord) : world(world), coord(coord) {}

        FractalWorld3D& world; // The world that this node is part of
        FractalRegion3D* region; // The region this node is contained in
        FractalCoord3D coord; // The position of this node in its world

        uint8_t num_observers = 0; // Number of observers looking at me
        Clock::time_point unload_time; // Time when number of observers hit 0. Can be used to unload chunk after an amount of time
        Clock::time_point last_update_time; // Time since last update to help with world merging by testing which chunk was updated most recently

        FractalNode3D* parent = nullptr; // Octree parent
        FractalNode3D* children[2][2][2] = { nullptr }; // Octree children
        FractalNode3D* neighbours[6] = { nullptr }; // Fast access of neighbours of same scale
    };
}