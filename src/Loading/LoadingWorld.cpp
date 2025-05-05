#include "LoadingWorld.h"

#include "Spatial3D/SpatialWorld.h"

namespace voxel_game::loading
{
	std::string_view LoadTask::ProcessFull(std::string_view key, std::string_view value)
	{
		finished = true;
		return NOOP;
	}

	std::string_view LoadTask::ProcessEmpty(std::string_view key)
	{
		finished = true;
		return NOOP;
	}

	void WorldOpenDatabase(Simulation& simulation, spatial3d::WorldPtr world, const godot::String& path)
	{
		tkrzw::Status status = (world->*&World::database).Open(std::string(path.utf8()), true, tkrzw::File::OPEN_NO_WAIT | tkrzw::File::OPEN_SYNC_HARD);

		if (status == tkrzw::Status::SUCCESS)
		{

		}
	}

	void ScaleDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		spatial3d::WorldPtr world = spatial3d::GetWorld(scale);

		DEBUG_ASSERT((world->*&World::database).IsOpen(), "The world should be open");

		spatial3d::ScaleDoNodeCommands(scale, spatial3d::NodeState::Loading,
			[&](spatial3d::NodePtr node, uint16_t& task_count)
		{
			switch(node->*&Node::state)
			{
			case NodeState::Unloaded:
				task_count++;
				node->*&Node::state = NodeState::Loading;
				break;

			case NodeState::Loading:
				if ((node->*&Node::task)->finished == true)
				{
					task_count--;
					node->*&Node::state = NodeState::Loaded;
				}
				break;
			}
		});
	}
}