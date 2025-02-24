#pragma once

#include "Util/Time.h"

namespace voxel_game
{
	struct CStamina
	{
		uint32_t max_stamina;
		uint32_t stamina;
	};

	struct CMovement
	{
		uint32_t speed;
	};

	struct CSwim
	{
		uint32_t swim_type;
		uint32_t swim_strength;
		uint32_t swim_speed;
	};

	struct CFlight
	{
		uint32_t flight_type;
		uint32_t fly_speed;
	};
}