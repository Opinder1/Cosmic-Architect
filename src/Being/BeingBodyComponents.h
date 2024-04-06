#pragma once

#include <cstdint>

namespace voxel_game
{
	struct BodyComponent
	{
		uint8_t fitness;
		uint8_t sight_distance;
		uint16_t strength;
		uint16_t body_size;
	};

	struct HumanoidComponent
	{
		uint32_t skin_colour;
		uint32_t facial_features;
		uint8_t height;
		uint8_t weight;
		uint8_t muscles;
		uint8_t gender;
	};

	struct VirtualBodyComponent
	{
		uint16_t virtual_presence;
	};

	struct SpecialEyesRelationship
	{

	};

	struct CyborgImplantRelationship
	{

	};

	struct EquipmentRelationship
	{

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
}