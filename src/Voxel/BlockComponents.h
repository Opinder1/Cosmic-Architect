#pragma once

#include <flecs/flecs.h>

namespace voxel_game::voxel
{
	struct BlockTypeComponent {};

	struct BlockMassComponent
	{
		uint32_t mass = 0;
	};

	struct BlockBreakableComponent
	{
		uint32_t tool_strength_required = 0;
	};

	struct BlockPlaceableComponent {};

	struct BlockToughnessComponent
	{
		uint32_t toughness = 0;
	};

	struct BlockResistanceComponent
	{
		uint32_t explosion_resistance = 0;
	};

	struct BlockCombustibleComponent
	{
		uint8_t burn_time = 0;
	};

	struct BlockDroppableComponent
	{
		flecs::entity_t drop_type = 0;
	};

	struct BlockLiquidComponent
	{
		uint32_t flow_rate = 0;
		uint32_t swim_resistance = 0;
	};

	struct BlockTemperatureComponent
	{
		int32_t temperature = 0;
	};

	struct BlockAfflicingComponent
	{
		flecs::entity_t effect = 0;
	};
}