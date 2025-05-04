#include "LoadingWorld.h"

#include "Spatial3D/SpatialWorld.h"

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

	void WorldOpenDatabase(Simulation& simulation, spatial3d::WorldPtr world, const godot::String path)
	{
		tkrzw::Status status = (world->*&World::database).Open(std::string(path.utf8()), true, tkrzw::File::OPEN_NO_WAIT | tkrzw::File::OPEN_SYNC_HARD);
	}

	void WorldDoCreateCommands(Simulation& simulation, spatial3d::WorldPtr world)
	{
	}

	void WorldDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale)
	{
		spatial3d::ScaleDoNodeCommands(scale, spatial3d::NodeState::Loading,
			[&](spatial3d::NodePtr node, uint16_t& task_count)
		{

		});
	}
}