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

	struct RotationComponent
	{
		real_t rotation;
	};

	struct ScaleComponent
	{
		real_t scale;
	};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct Position3DComponent
	{
		godot::Vector3 position;
	};

	struct Velocity3DComponent
	{
		godot::Vector3 velocity;
	};

	struct Rotation3DComponent
	{
		godot::Quaternion rotation;
	};

	struct Scale3DComponent
	{
		godot::Vector3 scale;
	};

	struct MassComponent
	{
		real_t mass = 0.0;
	};

	struct BouncinessComponent
	{
		real_t bounciness = 0.0;
	};

	struct RoughnessComponent
	{
		real_t roughness = 0.0;
	};

	struct Gravity3DComponent
	{
		godot::Vector3 force;
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
		real_t radius = 0.0;
	};
}