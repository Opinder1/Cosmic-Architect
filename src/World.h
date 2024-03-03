#pragma once

#include <flecs/flecs.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>

namespace voxel_world
{
	class World : public godot::RefCounted
	{
		GDCLASS(World, godot::RefCounted);

	public:
		World();
		~World();

		bool progress(double delta);

	protected:
		static void _bind_methods();

	private:
		flecs::world m_world;
	};

	class WorldNode : public godot::Node3D
	{
		GDCLASS(WorldNode, godot::Node3D);

	public:
		WorldNode();
		~WorldNode();

		void SetWorld(const godot::Ref<World>& world);
		godot::Ref<World> GetWorld();

		void _process(double delta) override;

	protected:
		static void _bind_methods();

	private:
		godot::Ref<World> m_world;
	};
}