#pragma once

#include <godot_cpp/variant/quaternion.hpp>

#include <flecs/flecs.h>

#include <vector>
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

	// Direction the entity is facing.
	struct CFacing
	{
		godot::Quaternion direction;
		float fov;
	};

	// How far the entity can sense either visually, vibration, smell or spiritually (6th sense)
	struct CSense
	{
		float sense_distance;
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

	// Special eyes that a being can have
	struct CSpecialEyes
	{

	};

	// An implant that a species has. Can have multiple
	struct CCyborgImplants
	{
		std::vector<flecs::entity_t> implants;
	};

	// An item being worn by a species. Can have multiple
	struct CEquipment
	{
		std::vector<flecs::entity_t> equipment;
	};
}