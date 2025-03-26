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

template<> const std::array<PolyComponentCB, 6> spatial3d::NodeType::k_type_constructors =
{
	PolyComponentConstruct<spatial3d::NodeType::Header>,
	PolyComponentConstruct<spatial3d::Node>,
	PolyComponentConstruct<universe::Node>,
	PolyComponentConstruct<galaxy::Node>,
	PolyComponentConstruct<voxel::Node>,
	PolyComponentConstruct<voxelrender::Node>,
};

template<> const std::array<PolyComponentCB, 6> spatial3d::NodeType::k_type_destructors =
{
	PolyComponentDestruct<spatial3d::NodeType::Header>,
	PolyComponentDestruct<spatial3d::Node>,
	PolyComponentDestruct<universe::Node>,
	PolyComponentDestruct<galaxy::Node>,
	PolyComponentDestruct<voxel::Node>,
	PolyComponentDestruct<voxelrender::Node>,
};

template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::ScaleType::Header> = 0;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::Scale> = 1;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<universe::Scale> = 2;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<galaxy::Scale> = 3;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxel::Scale> = 4;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxelrender::Scale> = 5;

template<> const std::array<PolyComponentCB, 6> spatial3d::ScaleType::k_type_constructors =
{
	PolyComponentConstruct<spatial3d::ScaleType::Header>,
	PolyComponentConstruct<spatial3d::Scale>,
	PolyComponentConstruct<universe::Scale>,
	PolyComponentConstruct<galaxy::Scale>,
	PolyComponentConstruct<voxel::Scale>,
	PolyComponentConstruct<voxelrender::Scale>,
};

template<> const std::array<PolyComponentCB, 6> spatial3d::ScaleType::k_type_destructors =
{
	PolyComponentDestruct<spatial3d::ScaleType::Header>,
	PolyComponentDestruct<spatial3d::Scale>,
	PolyComponentDestruct<universe::Scale>,
	PolyComponentDestruct<galaxy::Scale>,
	PolyComponentDestruct<voxel::Scale>,
	PolyComponentDestruct<voxelrender::Scale>,
};

template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::WorldType::Header> = 0;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::World> = 1;
template<> template<> const size_t spatial3d::WorldType::k_type_index<universe::World> = 2;
template<> template<> const size_t spatial3d::WorldType::k_type_index<galaxy::World> = 3;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxel::World> = 4;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxelrender::World> = 5;

template<> const std::array<PolyComponentCB, 6> spatial3d::WorldType::k_type_constructors =
{
	PolyComponentConstruct<spatial3d::WorldType::Header>,
	PolyComponentConstruct<spatial3d::World>,
	PolyComponentConstruct<universe::World>,
	PolyComponentConstruct<galaxy::World>,
	PolyComponentConstruct<voxel::World>,
	PolyComponentConstruct<voxelrender::World>,
};

template<> const std::array<PolyComponentCB, 6> spatial3d::WorldType::k_type_destructors =
{
	PolyComponentDestruct<spatial3d::WorldType::Header>,
	PolyComponentDestruct<spatial3d::World>,
	PolyComponentDestruct<universe::World>,
	PolyComponentDestruct<galaxy::World>,
	PolyComponentDestruct<voxel::World>,
	PolyComponentDestruct<voxelrender::World>,
};