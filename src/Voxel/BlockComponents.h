#pragma once

#include "Entity/EntityPoly.h"

#include <vector>

namespace voxel_game::voxel
{
	struct CBlockType {};

	struct CBlockMass
	{
		uint32_t mass = 0;
	};

	struct CBlockBreakable
	{
		uint32_t tool_strength_required = 0;
	};

	struct CBlockPlaceable {};

	struct CBlockToughness
	{
		uint32_t toughness = 0;
	};

	struct CBlockResistance
	{
		uint32_t explosion_resistance = 0;
	};

	struct CBlockCombustible
	{
		uint8_t burn_time = 0;
	};

	struct CBlockDroppable
	{
		entity::Ref drop_type;
	};

	struct CBlockLiquid
	{
		uint32_t flow_rate = 0;
		uint32_t swim_resistance = 0;
	};

	struct CBlockTemperature
	{
		int32_t temperature = 0;
	};

	struct CBlockAfflicing
	{
		entity::Ref effect;
	};
}