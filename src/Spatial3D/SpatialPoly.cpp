#include "SpatialPoly.h"

#include "SpatialWorld.h"
#include "Universe/UniverseWorld.h"
#include "Galaxy/GalaxyWorld.h"
#include "Voxel/VoxelWorld.h"
#include "VoxelRender/VoxelRenderWorld.h"

template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::spatial3d::NodeHeader> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::spatial3d::Node> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::universe::Node> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::galaxy::Node> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::voxel::Node> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::NodeHeader>::k_type_index<voxel_game::voxelrender::Node> = 4;

template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::spatial3d::ScaleHeader> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::spatial3d::Scale> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::universe::Scale> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::galaxy::Scale> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::voxel::Scale> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::ScaleHeader>::k_type_index<voxel_game::voxelrender::Scale> = 4;

template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::spatial3d::WorldHeader> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::spatial3d::World> = 0;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::universe::World> = 1;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::galaxy::World> = 2;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::voxel::World> = 3;
template<> template<> const size_t PolyType<voxel_game::spatial3d::WorldHeader>::k_type_index<voxel_game::voxelrender::World> = 4;