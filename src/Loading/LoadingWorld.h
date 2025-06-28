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

	enum class TaskState
	{
		Unloaded,
		Loaded,
		Loading,
		Saving,
		Unloading
	};

	struct Task : tkrzw::DBM::RecordProcessor
	{
		TaskState state = TaskState::Unloaded;
	};

	struct LoadTask : Task
	{
		std::string_view ProcessFull(std::string_view key, std::string_view value) override;

		std::string_view ProcessEmpty(std::string_view key) override;
	};

	struct SaveTask : Task
	{
		std::string_view ProcessFull(std::string_view key, std::string_view value) override;

		std::string_view ProcessEmpty(std::string_view key) override;
	};

	struct UnloadTask : Task
	{
		std::string_view ProcessFull(std::string_view key, std::string_view value) override;

		std::string_view ProcessEmpty(std::string_view key) override;
	};

	struct Node
	{
		std::unique_ptr<Task> task;
	};

	struct Scale
	{

	};

	// This is a specialised world that loads nodes from a database
	struct World
	{
		tkrzw::ShardDBM database; // Database to load nodes from

		std::vector<LoadFunc> node_post_read_func; // Functions to intepret node data by each node component
	};

	void WorldOpenDatabase(Simulation& simulation, spatial3d::WorldPtr world, const godot::String& path);

	void WorldCloseDatabase(Simulation& simulation, spatial3d::WorldPtr world);

	void ScaleDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale);
}