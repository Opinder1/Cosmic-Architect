#pragma once

#include <bitset>

namespace voxel_game
{
	struct SentienceComponent
	{
		uint8_t personality_type;
		uint8_t max_saneness;
		uint8_t saneness;
		uint8_t intelligence;
		uint8_t pridefulness;
	};

	struct SentienceRelationship
	{
		std::bitset<32> personality_traits;
	};

	struct LanguageRelationship
	{
		uint8_t proficiency;
	};

	struct SkillRelationship
	{
		float skill_proficiency;
	};

	struct ReproductiveComponent
	{
		uint8_t gender;
		uint8_t urge;
	};

	struct BeliefRelationship
	{
		uint8_t belief;
	};

	struct BeingRelationship
	{
		std::bitset<32> thoughts;
	};
}