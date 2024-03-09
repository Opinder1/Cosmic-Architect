#pragma once

#include <flecs/flecs.h>

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace voxel_world
{
	class World : public godot::Resource
	{
		GDCLASS(World, godot::Resource);

	public:
		World();
		~World();

		void Reset();

		void StartRest(uint16_t port, bool monitor);

		void SetThreads(int64_t threads);

		bool Progress(double delta);

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

		// Load a new world and make this node the owner
		void LoadWorld(const godot::String& path);

		void _process(double delta) override;

	protected:
		static void _bind_methods();

	private:
		godot::Ref<World> m_world;

		bool m_owner;
	};
}