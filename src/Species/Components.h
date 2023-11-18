#pragma once

#include "Util/Time.h"

#include <entt/fwd.hpp>

#include <vector>
#include <bitset>

struct HumanoidComponent
{
	uint32_t height;
	uint32_t skin_colour;
	uint32_t facial_features;
};

struct BeastComponent
{

};

struct AquaticComponent
{

};

struct DraconicComponent
{

};

struct DemonicComponent
{

};

struct UndeadComponent
{

};

struct VampyricComponent
{

};

struct MechanicalLifeformComponent
{

};

struct BioechanicalLifeformComponent
{

};

struct GaseousLifeformComponent
{

};

struct EnergyLifeformComponent
{

};

struct VirtualLifeformComponent
{

};

struct RaceBodyComponent
{
	uint32_t raw_strength;
	uint32_t raw_speed;
	uint32_t raw_stamina;
	uint32_t raw_sight_distance;
	uint32_t body_size;
};

struct RaceAquaticComponent
{
	uint32_t swim_type;
	uint32_t raw_swim_strength;
	uint32_t raw_swim_speed;
};

struct RaceBreathComponent
{
	uint32_t median_breath;
	bool breathe_underwater;
	bool breathe_space;
	bool breathe_toxic;
	bool breathe_magma;
};

struct RaceFlightComponent
{
	uint32_t flight_type;
	uint32_t median_fly_speed;
};

struct RaceMentalComponent
{
	uint32_t median_max_saneness;
	uint32_t median_intelligence;
};

struct RaceSentienceComponent
{
	uint32_t personality_type;
	std::bitset<64> possible_personality_traits;
	std::vector<float> trait_likelyhood;
};

struct RaceLifespanComponent
{
	Clock::duration median_lifespan;
};

struct RaceLanguageComponent
{
	entt::entity instinctual_language;
};

struct RaceHealthComponent
{
	uint32_t median_health;
};

struct RaceHungerComponent
{
	uint32_t median_hunger;
	std::vector<entt::entity> required_food_types;
	std::vector<entt::entity> preferred_food_types;
};

struct RaceThirstComponent
{
	uint32_t median_thirst;
	std::vector<entt::entity> required_drink_types;
	std::vector<entt::entity> preferred_drink_types;
};

struct RaceStaminaComponent
{
	uint32_t median_stamina;
};

struct RaceSkillComponent
{
	std::vector<entt::entity> possible_race_skills;
	std::vector<float> skill_likelyhood;
};

struct RaceReproductiveComponent
{

};

struct RaceResistanceComponent
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

	std::vector<uint8_t> status_resistance_types; // (Could just turn into components)
	std::vector<uint8_t> median_status_resistances;
};