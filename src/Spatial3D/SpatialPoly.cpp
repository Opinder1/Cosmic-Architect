#include "SpatialPoly.h"

#include "SpatialWorld.h"
#include "Universe/UniverseWorld.h"
#include "Galaxy/GalaxyWorld.h"
#include "Voxel/VoxelWorld.h"
#include "VoxelRender/VoxelRenderWorld.h"

using namespace voxel_game;

template<> template<> const size_t spatial3d::NodeType::k_type_index<spatial3d::NodeType::Header> = 0;
template<> template<> const size_t spatial3d::NodeType::k_type_index<spatial3d::Node> = 1;
template<> template<> const size_t spatial3d::NodeType::k_type_index<universe::Node> = 2;
template<> template<> const size_t spatial3d::NodeType::k_type_index<galaxy::Node> = 3;
template<> template<> const size_t spatial3d::NodeType::k_type_index<voxel::Node> = 4;
template<> template<> const size_t spatial3d::NodeType::k_type_index<voxelrender::Node> = 5;

const std::array<PolyTypeInfo, 6> PolyType<spatial3d::NodeType, 6>::k_type_info =
{
	MakeTypeInfo<spatial3d::NodeType::Header>(),
	MakeTypeInfo<spatial3d::Node>(),
	MakeTypeInfo<universe::Node>(),
	MakeTypeInfo<galaxy::Node>(),
	MakeTypeInfo<voxel::Node>(),
	MakeTypeInfo<voxelrender::Node>(),
};

template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::ScaleType::Header> = 0;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::Scale> = 1;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<universe::Scale> = 2;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<galaxy::Scale> = 3;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxel::Scale> = 4;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxelrender::Scale> = 5;

const std::array<PolyTypeInfo, 6> PolyType<spatial3d::ScaleType, 6>::k_type_info =
{
	MakeTypeInfo<spatial3d::ScaleType::Header>(),
	MakeTypeInfo<spatial3d::Scale>(),
	MakeTypeInfo<universe::Scale>(),
	MakeTypeInfo<galaxy::Scale>(),
	MakeTypeInfo<voxel::Scale>(),
	MakeTypeInfo<voxelrender::Scale>(),
};

template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::WorldType::Header> = 0;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::World> = 1;
template<> template<> const size_t spatial3d::WorldType::k_type_index<universe::World> = 2;
template<> template<> const size_t spatial3d::WorldType::k_type_index<galaxy::World> = 3;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxel::World> = 4;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxelrender::World> = 5;

const std::array<PolyTypeInfo, 6> PolyType<spatial3d::WorldType, 6>::k_type_info =
{
	MakeTypeInfo<spatial3d::WorldType::Header>(),
	MakeTypeInfo<spatial3d::World>(),
	MakeTypeInfo<universe::World>(),
	MakeTypeInfo<galaxy::World>(),
	MakeTypeInfo<voxel::World>(),
	MakeTypeInfo<voxelrender::World>(),
};