#pragma once

#include "Spatial3D/SpatialPoly.h"

#include <TKRZW/tkrzw_dbm_shard.h>

#include <deque>
#include <vector>

namespace voxel_game
{
	struct Simulation;
}

namespace voxel_game::loading
{
	using LoadFunc = void(*)(Simulation& simulation, spatial3d::WorldPtr world, spatial3d::NodePtr node);

	struct LoadTask : tkrzw::DBM::RecordProcessor
	{
		std::string_view ProcessFull(std::string_view key, std::string_view value) override;

		std::string_view ProcessEmpty(std::string_view key) override;
	};

	struct Scale
	{
		std::deque<LoadTask> node_load_tasks;
	};

	// This is a specialised world that loads nodes from a database
	struct World
	{
		tkrzw::ShardDBM database; // Database to load nodes from

		std::vector<LoadFunc> node_post_read_func; // Functions to intepret node data by each node component
	};

	void WorldOpenDatabase(Simulation& simulation, spatial3d::WorldPtr world, const godot::String path);

	void WorldDoCreateCommands(Simulation& simulation, spatial3d::WorldPtr world);

	void WorldDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale);
}