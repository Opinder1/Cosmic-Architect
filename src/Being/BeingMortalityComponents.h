#pragma once

#include "Util/Time.h"

namespace voxel_game
{
	struct BirthTimeComponent
	{
		Clock::time_point birth_time;
	};

	struct LifespanComponent
	{
		Clock::duration lifespan;
	};

	struct HungerComponent
	{
		uint16_t stomach_size;
		uint16_t hunger;
		uint16_t hunger_loss_rate;
	};

	struct ThirstComponent
	{
		uint8_t thirst_max;
		uint8_t thirst;
		uint8_t thirst_loss_rate;
	};

	struct StaminaComponent
	{
		uint8_t stamina_max;
		uint8_t stamina;
		uint8_t stamina_loss_rate;
	};

	struct BreathComponent
	{
		uint8_t breath;
		bool breathe_underwater : 1;
		bool breathe_space : 1;
		bool breathe_toxic : 1;
		bool breathe_magma : 1;
	};

	struct HealthComponent
	{
		uint16_t health_max;
		uint16_t health;
	};

	struct StatusEffectRelationship
	{
		uint8_t effect_amount;
	};

	struct ResistanceComponent
	{
		uint8_t physical_resistance;
		uint8_t stab_resistance;
		uint8_t blunt_resistance;
		uint8_t cut_resistance;
		uint8_t concussion_resistance;
		uint8_t bleed_resistance;

		uint8_t fire_resistance;
		uint8_t ice_resistance;
		uint8_t heat_resistance;
		uint8_t cold_resistance;
		uint8_t lightning_resistance;

		uint8_t poison_resistance;
		uint8_t plasma_resistance;
		uint8_t radiation_resistance;
		uint8_t sound_resistance;
		uint8_t vaccum_resistance;

		uint8_t qi_resistance;
		uint8_t magic_resistance;
		uint8_t quantum_resistance;
		uint8_t mind_resistance;
		uint8_t soul_resistance;
		uint8_t divinity_resistance;
		uint8_t chaos_resistance;
	};
}