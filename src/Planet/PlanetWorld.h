#pragma once

namespace voxel_game::planet
{
	struct Node : spatial3d::Node, voxelrender::Node, civilization::Node, factions::Node, climate::Node, ecology::Node, energy::Node, pathfind::Node, collision::Node, scriptlogic::Node
	{

	};

	struct Scale : spatial3d::Scale, voxelrender::Scale, civilization::Scale, factions::Scale, climate::Scale, ecology::Scale, energy::Scale, pathfind::Scale, collision::Scale, scriptlogic::Scale
	{

	};
}