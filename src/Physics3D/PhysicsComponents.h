#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/Quaternion.hpp>
#include <godot_cpp/variant/AABB.hpp>

namespace flecs
{
	struct world;
}

namespace voxel_game::physics3d
{
	struct Components
	{
		Components(flecs::world& world);
	};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct Position
	{
		godot::Vector3 position;
	};

	struct Velocity
	{
		godot::Vector3 velocity;
	};

	struct Rotation
	{
		godot::Quaternion rotation;
	};

	struct Scale
	{
		godot::Vector3 scale;
	};

	struct Mass
	{
		real_t mass = 0.0;
	};

	struct Bounciness
	{
		real_t bounciness = 0.0;
	};

	struct Roughness
	{
		real_t roughness = 0.0;
	};

	struct Gravity
	{
		godot::Vector3 force;
	};

	struct AABB
	{
		godot::AABB aabb;
	};

	struct Box
	{
		godot::Vector3 size;
	};

	struct Sphere
	{
		real_t radius = 0.0;
	};
}