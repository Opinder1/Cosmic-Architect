#pragma once

#include <bitset>

namespace voxel_game
{
	struct CSentience
	{
		uint8_t personality_type;
		std::bitset<32> personality_traits;
		uint8_t max_saneness;
		uint8_t saneness;
		uint8_t intelligence;
		uint8_t pridefulness;
	};

	// Relationship to a language entity. Can have multiple
	struct RLanguage
	{
		uint8_t proficiency;
	};

	// Relationship to a skill entity. Can have multiple
	struct RSkill
	{
		float skill_proficiency;
	};

	struct CReproductive
	{
		uint8_t gender;
		uint8_t urge;
	};

	// Relationship to a religion entity. Can have multiple
	struct RBelief
	{
		uint8_t belief;
	};

	// Relationship to another being entity. Can have multiple
	struct RBeing
	{
		std::bitset<32> thoughts;
	};
}