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

		void StartRest(uint64_t port, bool monitor);

		void SetThreads(uint64_t threads);

		void Reset();

		void ResetTime();

		bool Progress(double delta);

		float GetDeltaTime();

		void PreallocateForEntities(uint64_t entity_count);

		void SetEntityRange(uint64_t min, uint64_t max);

		void SetEntityRangeCheck(bool enabled);

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