#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/Quaternion.hpp>
#include <godot_cpp/variant/AABB.hpp>

namespace flecs
{
	struct world;
}

namespace voxel_game
{
	struct PhysicsComponents
	{
		PhysicsComponents(flecs::world& world);
	};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct Position3DComponent
	{
		godot::Vector3 position;
	};

	struct Rotation3DComponent
	{
		godot::Quaternion rotation;
	};

	struct Velocity3DComponent
	{
		godot::Vector3 velocity;
	};

	struct MassComponent
	{
		double mass = 0.0;
	};

	struct BouncinessComponent
	{
		double bounciness = 0.0;
	};

	struct RoughnessComponent
	{
		double roughness = 0.0;
	};

	struct AABBComponent
	{
		godot::AABB aabb;
	};

	struct Box3DComponent
	{
		godot::Vector3 size;
	};

	struct SphereComponent
	{
		double radius = 0.0;
	};
}