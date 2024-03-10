#pragma once

#include "World.h"

#include <godot_cpp/classes/node3d.hpp>

namespace voxel_world
{
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