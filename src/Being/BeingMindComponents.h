#pragma once

#include <bitset>

namespace voxel_game
{
	struct CSentience
	{
		uint8_t personality_type;
		uint8_t max_saneness;
		uint8_t saneness;
		uint8_t intelligence;
		uint8_t pridefulness;
	};

	struct RSentience
	{
		std::bitset<32> personality_traits;
	};

	struct RLanguage
	{
		uint8_t proficiency;
	};

	struct RSkill
	{
		float skill_proficiency;
	};

	struct CReproductive
	{
		uint8_t gender;
		uint8_t urge;
	};

	struct RBelief
	{
		uint8_t belief;
	};

	struct RBeing
	{
		std::bitset<32> thoughts;
	};
}