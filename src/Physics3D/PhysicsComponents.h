#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/Quaternion.hpp>
#include <godot_cpp/variant/AABB.hpp>

#include <flecs/flecs.h>

namespace voxel_game::physics3d
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct CPosition
	{
		godot::Vector3 position;
	};

	struct CVelocity
	{
		godot::Vector3 velocity;
	};

	struct CRotation
	{
		godot::Quaternion rotation;
	};

	struct CScale
	{
		godot::Vector3 scale;
	};

	struct CMass
	{
		real_t mass = 0.0;
	};

	struct CBounciness
	{
		real_t bounciness = 0.0;
	};

	struct CRoughness
	{
		real_t roughness = 0.0;
	};

	struct CGravity
	{
		godot::Vector3 force;
	};

	struct CAABB
	{
		godot::AABB aabb;
	};

	struct CBox
	{
		godot::Vector3 size;
	};

	struct CSphere
	{
		real_t radius = 0.0;
	};
}