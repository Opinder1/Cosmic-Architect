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

const std::array<ComponentConstructCB, 6> PolyType<spatial3d::NodeType, 6>::k_type_constructors =
{
	ComponentConstruct<spatial3d::NodeType::Header>,
	ComponentConstruct<spatial3d::Node>,
	ComponentConstruct<universe::Node>,
	ComponentConstruct<galaxy::Node>,
	ComponentConstruct<voxel::Node>,
	ComponentConstruct<voxelrender::Node>,
};

const std::array<ComponentDestructCB, 6> PolyType<spatial3d::NodeType, 6>::k_type_destructors =
{
	ComponentDestruct<spatial3d::NodeType::Header>,
	ComponentDestruct<spatial3d::Node>,
	ComponentDestruct<universe::Node>,
	ComponentDestruct<galaxy::Node>,
	ComponentDestruct<voxel::Node>,
	ComponentDestruct<voxelrender::Node>,
};

const std::array<ComponentMoveCB, 6> PolyType<spatial3d::NodeType, 6>::k_type_movers =
{
	ComponentMove<spatial3d::NodeType::Header>,
	nullptr,
	ComponentMove<universe::Node>,
	ComponentMove<galaxy::Node>,
	ComponentMove<voxel::Node>,
	ComponentMove<voxelrender::Node>,
};

const std::array<size_t, 6> PolyType<spatial3d::NodeType, 6>::k_type_sizes =
{
	sizeof(spatial3d::NodeType::Header),
	sizeof(spatial3d::Node),
	sizeof(universe::Node),
	sizeof(galaxy::Node),
	sizeof(voxel::Node),
	sizeof(voxelrender::Node),
};

template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::ScaleType::Header> = 0;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<spatial3d::Scale> = 1;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<universe::Scale> = 2;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<galaxy::Scale> = 3;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxel::Scale> = 4;
template<> template<> const size_t spatial3d::ScaleType::k_type_index<voxelrender::Scale> = 5;

const std::array<ComponentConstructCB, 6> PolyType<spatial3d::ScaleType, 6>::k_type_constructors =
{
	ComponentConstruct<spatial3d::ScaleType::Header>,
	ComponentConstruct<spatial3d::Scale>,
	ComponentConstruct<universe::Scale>,
	ComponentConstruct<galaxy::Scale>,
	ComponentConstruct<voxel::Scale>,
	ComponentConstruct<voxelrender::Scale>,
};

const std::array<ComponentDestructCB, 6> PolyType<spatial3d::ScaleType, 6>::k_type_destructors =
{
	ComponentDestruct<spatial3d::ScaleType::Header>,
	ComponentDestruct<spatial3d::Scale>,
	ComponentDestruct<universe::Scale>,
	ComponentDestruct<galaxy::Scale>,
	ComponentDestruct<voxel::Scale>,
	ComponentDestruct<voxelrender::Scale>,
};

const std::array<ComponentMoveCB, 6> PolyType<spatial3d::ScaleType, 6>::k_type_movers =
{
	ComponentMove<spatial3d::ScaleType::Header>,
	nullptr,
	ComponentMove<universe::Scale>,
	ComponentMove<galaxy::Scale>,
	ComponentMove<voxel::Scale>,
	ComponentMove<voxelrender::Scale>,
};

const std::array<size_t, 6> PolyType<spatial3d::ScaleType, 6>::k_type_sizes =
{
	sizeof(spatial3d::ScaleType::Header),
	sizeof(spatial3d::Scale),
	sizeof(universe::Scale),
	sizeof(galaxy::Scale),
	sizeof(voxel::Scale),
	sizeof(voxelrender::Scale),
};

template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::WorldType::Header> = 0;
template<> template<> const size_t spatial3d::WorldType::k_type_index<spatial3d::World> = 1;
template<> template<> const size_t spatial3d::WorldType::k_type_index<universe::World> = 2;
template<> template<> const size_t spatial3d::WorldType::k_type_index<galaxy::World> = 3;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxel::World> = 4;
template<> template<> const size_t spatial3d::WorldType::k_type_index<voxelrender::World> = 5;

const std::array<ComponentConstructCB, 6> PolyType<spatial3d::WorldType, 6>::k_type_constructors =
{
	ComponentConstruct<spatial3d::WorldType::Header>,
	ComponentConstruct<spatial3d::World>,
	ComponentConstruct<universe::World>,
	ComponentConstruct<galaxy::World>,
	ComponentConstruct<voxel::World>,
	ComponentConstruct<voxelrender::World>,
};

const std::array<ComponentDestructCB, 6> PolyType<spatial3d::WorldType, 6>::k_type_destructors =
{
	ComponentDestruct<spatial3d::WorldType::Header>,
	ComponentDestruct<spatial3d::World>,
	ComponentDestruct<universe::World>,
	ComponentDestruct<galaxy::World>,
	ComponentDestruct<voxel::World>,
	ComponentDestruct<voxelrender::World>,
};

const std::array<ComponentMoveCB, 6> PolyType<spatial3d::WorldType, 6>::k_type_movers =
{
	ComponentMove<spatial3d::WorldType::Header>,
	nullptr,
	ComponentMove<universe::World>,
	ComponentMove<galaxy::World>,
	ComponentMove<voxel::World>,
	ComponentMove<voxelrender::World>,
};

const std::array<size_t, 6> PolyType<spatial3d::WorldType, 6>::k_type_sizes =
{
	sizeof(spatial3d::WorldType::Header),
	sizeof(spatial3d::World),
	sizeof(universe::World),
	sizeof(galaxy::World),
	sizeof(voxel::World),
	sizeof(voxelrender::World),
};