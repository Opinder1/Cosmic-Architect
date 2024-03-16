#pragma once

#include "FlecsWorld.h"

#include <godot_cpp/classes/node3d.hpp>

namespace voxel_game
{
	class FlecsWorldNode : public godot::Node3D
	{
		GDCLASS(FlecsWorldNode, godot::Node3D);

	public:
		FlecsWorldNode();
		~FlecsWorldNode();

		void SetWorld(const godot::Ref<FlecsWorld>& world);
		godot::Ref<FlecsWorld> GetWorld();

	protected:
		static void _bind_methods();

	private:
		godot::Ref<FlecsWorld> m_world;
	};
}