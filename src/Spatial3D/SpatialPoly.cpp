#include "SpatialPoly.h"

#include "SpatialWorld.h"
#include "Universe/UniverseWorld.h"
#include "Galaxy/GalaxyWorld.h"
#include "Voxel/VoxelWorld.h"
#include "VoxelRender/VoxelRenderWorld.h"

template<> template<> const size_t PolyType<voxel_game::spatial3d::NodePolyType, 6>::k_type_index<voxel_game::spatial3d::Node> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodePolyType, 6>::k_type_index<voxel_game::universe::Node> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodePolyType, 6>::k_type_index<voxel_game::galaxy::Node> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodePolyType, 6>::k_type_index<voxel_game::voxel::Node> = 4;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodePolyType, 6>::k_type_index<voxel_game::voxelrender::Node> = 5;

template<> template<> const size_t PolyType<voxel_game::spatial3d::ScalePolyType, 6>::k_type_index<voxel_game::spatial3d::Scale> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScalePolyType, 6>::k_type_index<voxel_game::universe::Scale> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScalePolyType, 6>::k_type_index<voxel_game::galaxy::Scale> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScalePolyType, 6>::k_type_index<voxel_game::voxel::Scale> = 4;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScalePolyType, 6>::k_type_index<voxel_game::voxelrender::Scale> = 5;

template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldPolyType, 6>::k_type_index<voxel_game::spatial3d::World> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldPolyType, 6>::k_type_index<voxel_game::universe::World> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldPolyType, 6>::k_type_index<voxel_game::galaxy::World> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldPolyType, 6>::k_type_index<voxel_game::voxel::World> = 4;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldPolyType, 6>::k_type_index<voxel_game::voxelrender::World> = 5;