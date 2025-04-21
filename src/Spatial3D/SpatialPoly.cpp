#include "SpatialPoly.h"

#include "SpatialWorld.h"
#include "Loading/LoadingWorld.h"
#include "Universe/UniverseWorld.h"
#include "Galaxy/GalaxyWorld.h"
#include "Voxel/VoxelWorld.h"
#include "VoxelRender/VoxelRenderWorld.h"

using namespace voxel_game;

const size_t node_first = __LINE__ + 1;
template<> template<> const size_t spatial3d::NodeType::k_type_index<spatial3d::NodeType::Header>	= __LINE__ - node_first;
template<> template<> const size_t spatial3d::NodeType::k_type_index<spatial3d::Node>				= __LINE__ - node_first;
template<> template<> const size_t spatial3d::NodeType::k_type_index<universe::Node>				= __LINE__ - node_first;
template<> template<> const size_t spatial3d::NodeType::k_type_index<galaxy::Node>					= __LINE__ - node_first;
template<> template<> const size_t spatial3d::NodeType::k_type_index<voxel::Node>					= __LINE__ - node_first;
template<> template<> const size_t spatial3d::NodeType::k_type_index<voxelrender::Node>				= __LINE__ - node_first;

const std::array<PolyTypeInfo, 6> PolyType<spatial3d::NodeType, 6>::k_type_info =
{
	MakeTypeInfo<spatial3d::NodeType::Header>(),
	MakeTypeInfo<spatial3d::Node>(),
	MakeTypeInfo<universe::Node>(),
	MakeTypeInfo<galaxy::Node>(),
	MakeTypeInfo<voxel::Node>(),
	MakeTypeInfo<voxelrender::Node>(),
};

const size_t scale_first = __LINE__ + 1;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::ScaleType::Header> = __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::Scale>				= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::PartialScale>		= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<loading::Scale>				= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<universe::Scale>				= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<galaxy::Scale>				= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxel::Scale>					= __LINE__ - scale_first;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxelrender::Scale>			= __LINE__ - scale_first;

const std::array<PolyTypeInfo, 8> PolyType<spatial3d::ScaleType, 8>::k_type_info =
{
	MakeTypeInfo<spatial3d::ScaleType::Header>(),
	MakeTypeInfo<spatial3d::Scale>(),
	MakeTypeInfo<spatial3d::PartialScale>(),
	MakeTypeInfo<loading::Scale>(),
	MakeTypeInfo<universe::Scale>(),
	MakeTypeInfo<galaxy::Scale>(),
	MakeTypeInfo<voxel::Scale>(),
	MakeTypeInfo<voxelrender::Scale>(),
};

const size_t world_first = __LINE__ + 1;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::WorldType::Header> = __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::World>				= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::BoundedWorld>		= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::PartialWorld>		= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<loading::World>				= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<universe::World>				= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<galaxy::World>				= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxel::World>					= __LINE__ - world_first;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxelrender::World>			= __LINE__ - world_first;

const std::array<PolyTypeInfo, 9> PolyType<spatial3d::WorldType, 9>::k_type_info =
{
	MakeTypeInfo<spatial3d::WorldType::Header>(),
	MakeTypeInfo<spatial3d::World>(),
	MakeTypeInfo<spatial3d::BoundedWorld>(),
	MakeTypeInfo<spatial3d::PartialWorld>(),
	MakeTypeInfo<loading::World>(),
	MakeTypeInfo<universe::World>(),
	MakeTypeInfo<galaxy::World>(),
	MakeTypeInfo<voxel::World>(),
	MakeTypeInfo<voxelrender::World>(),
};