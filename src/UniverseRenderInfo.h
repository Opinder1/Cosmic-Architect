#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace voxel_game
{
	class UniverseRenderInfo : public godot::Object
	{
		GDCLASS(UniverseRenderInfo, godot::Object);

	public:
		UniverseRenderInfo();

		void SetScenario(godot::RID scenario);

		godot::RID GetScenario() const;

	public:
		static void _bind_methods();

	private:
		godot::RID m_scenario;
	};
}