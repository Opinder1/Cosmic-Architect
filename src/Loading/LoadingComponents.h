#pragma once

#include <cstdint>

namespace voxel_game
{
	struct Loadable
	{
		uint32_t children_to_load;
		uint32_t children_loaded;
	};

	struct Load
	{

	};
}