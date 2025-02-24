#pragma once

#include <cstdint>

namespace voxel_game
{
	struct CBody
	{
		uint8_t fitness;
		uint8_t sight_distance;
		uint16_t strength;
		uint16_t body_size;
	};

	struct CHumanoid
	{
		uint32_t skin_colour;
		uint32_t facial_features;
		uint8_t height;
		uint8_t weight;
		uint8_t muscles;
		uint8_t gender;
	};

	struct CVirtualBody
	{
		uint16_t virtual_presence;
	};

	struct RSpecialEyes
	{

	};

	struct RCyborgImplant
	{

	};

	struct REquipment
	{

	};

	struct CBeast
	{

	};

	struct CAquatic
	{

	};

	struct CDraconic
	{

	};

	struct CDemonic
	{

	};

	struct CUndead
	{

	};

	struct CVampyric
	{

	};

	struct CMechanicalLifeform
	{

	};

	struct CBioechanicalLifeform
	{

	};

	struct CGaseousLifeform
	{

	};

	struct CEnergyLifeform
	{

	};

	struct CVirtualLifeform
	{

	};
}