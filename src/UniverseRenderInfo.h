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

		void SetGalaxyScenario(godot::RID scenario);
		void SetStarScenario(godot::RID scenario);
		void SetMainScenario(godot::RID scenario);

		godot::RID GetGalaxyScenario() const;
		godot::RID GetStarScenario() const;
		godot::RID GetMainScenario() const;

	public:
		static void _bind_methods();

	private:
		godot::RID m_galaxy_scenario;
		godot::RID m_star_scenario;
		godot::RID m_main_scenario;
	};
}