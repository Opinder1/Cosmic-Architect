#include "LoadingWorld.h"

#include "Spatial3D/SpatialWorld.h"

#include <godot_cpp/classes/project_settings.hpp>

namespace voxel_game::loading
{
	std::string_view LoadTask::ProcessFull(std::string_view key, std::string_view value)
	{
		return NOOP;
	}

	std::string_view LoadTask::ProcessEmpty(std::string_view key)
	{
		return NOOP;
	}

	void WorldOpenDatabase(Simulation& simulation, spatial3d::WorldPtr world, const godot::String& path)
	{
		std::map<std::string, std::string> params;

		params.emplace("num_shards", "4");
		params.emplace("dbm", "HashDBM");

		int32_t options = tkrzw::File::OPEN_NO_WAIT | tkrzw::File::OPEN_SYNC_HARD;

		std::string os_path = godot::ProjectSettings::get_singleton()->globalize_path(path).utf8();

		tkrzw::Status status = (world->*&World::database).OpenAdvanced(os_path, true, options, params);

		if (status != tkrzw::Status::SUCCESS)
		{

		}
	}

	void WorldCloseDatabase(Simulation& simulation, spatial3d::WorldPtr world)
	{
		(world->*&World::database).Close();
	}

	void ScaleDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		spatial3d::WorldPtr world = spatial3d::GetWorld(scale);

		DEBUG_ASSERT((world->*&World::database).IsOpen(), "The world should be open");

		spatial3d::ScaleDoNodeCommands(scale, spatial3d::NodeState::Loading,
			[&](spatial3d::NodePtr node)
		{
			if (node->*&Node::task == nullptr)
			{	
				node->*&Node::task = std::make_unique<Task>();
				(node->*&Node::task)->state = TaskState::Loading;
				(node->*&spatial3d::PartialNode::task_count)++;
			}

			Task& task = *(node->*&Node::task);

			switch(task.state)
			{
			case TaskState::Loading:
				task.state = TaskState::Loaded;
				break;

			case TaskState::Loaded:
				(node->*&spatial3d::PartialNode::task_count)--;
				break;
			}

			return false;
		});
	}
}