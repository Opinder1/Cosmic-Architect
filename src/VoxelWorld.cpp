#include "VoxelWorld.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace voxel_world
{
	VoxelRegion::VoxelRegion(VoxelWorld& world) :
		FractalRegion3D(world),
		m_world(world)
	{

	}

	VoxelRegion::~VoxelRegion()
	{

	}

	FractalNode3D* VoxelRegion::GenerateNode(FractalCoord3D coord)
	{
		return memnew(VoxelNode(m_world, coord));
	}

	void VoxelRegion::OnNodeUpdate(FractalNode3D& node)
	{

	}

	void VoxelRegion::OnNodeUnload(FractalNode3D& node)
	{

	}


	VoxelWorld::VoxelWorld(sim::UUID owning_simulation) :
		FractalWorld3D(owning_simulation)
	{

	}

	VoxelWorld::~VoxelWorld()
	{

	}

	Block VoxelWorld::GetBaseBlock(const godot::Vector3i& pos) const
	{
		return Block{};
	}

	Block VoxelWorld::GetLODBlock(const FractalCoord3D& coord) const
	{
		return Block{};
	}

	FractalRegion3D* VoxelWorld::GenerateRegion()
	{
		return memnew(VoxelRegion(*this));
	}

	VoxelWorldNode::VoxelWorldNode()
	{
		m_root_simulation = sim::SimulationServer::GetSingleton()->CreateSimulation(60, true);

		godot::UtilityFunctions::print("Created VoxelWorldNode");
	}

	VoxelWorldNode::~VoxelWorldNode()
	{
		godot::UtilityFunctions::print("Destroyed VoxelWorldNode");
	}

	void VoxelWorldNode::_bind_methods()
	{

	}
}