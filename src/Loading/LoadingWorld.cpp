#include "LoadingWorld.h"

#include "Spatial3D/SpatialWorld.h"

namespace voxel_game::loading
{
	std::string_view UUIDToData(const UUID& id)
	{
		return std::string_view((const char*)&id, sizeof(UUID));
	}

	std::string_view NodePosToData(const godot::Vector3i pos)
	{
		return std::string_view((const char*)&pos, sizeof(godot::Vector3i));
	}

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
		DEBUG_ASSERT(world->*&spatial3d::World::max_scale > 0, "The spatial world should have at least one scale");

		for (size_t scale_index = 0; scale_index < world->*&spatial3d::World::max_scale; scale_index++)
		{
			spatial3d::ScalePtr scale = spatial3d::GetScale(world, scale_index);

			// For each create command
			for (const spatial3d::NodeCreateCommand& command : scale->*&spatial3d::PartialScale::create_commands)
			{
				(world->*&World::database).Process(NodePosToData(command.pos), &(scale->*&Scale::node_load_tasks).emplace_back(), true);
			}
		}
	}

	void WorldDoLoadCommands(Simulation& simulation, spatial3d::ScalePtr scale)
	{

	}
}