#include "UniverseRenderInfo.h"

#include "Util/PropertyMacros.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	UniverseRenderInfo::UniverseRenderInfo()
	{

	}

	void UniverseRenderInfo::SetGalaxyScenario(godot::RID scenario)
	{
		m_galaxy_scenario = scenario;
	}

	void UniverseRenderInfo::SetStarScenario(godot::RID scenario)
	{
		m_star_scenario = scenario;
	}

	void UniverseRenderInfo::SetMainScenario(godot::RID scenario)
	{
		m_main_scenario = scenario;
	}

	godot::RID UniverseRenderInfo::GetGalaxyScenario() const
	{
		return m_galaxy_scenario;
	}

	godot::RID UniverseRenderInfo::GetStarScenario() const
	{
		return m_star_scenario;
	}

	godot::RID UniverseRenderInfo::GetMainScenario() const
	{
		return m_main_scenario;
	}

	void UniverseRenderInfo::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_galaxy_scenario", "scenario"), &UniverseRenderInfo::SetGalaxyScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("set_star_scenario", "scenario"), &UniverseRenderInfo::SetStarScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("set_main_scenario", "scenario"), &UniverseRenderInfo::SetMainScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("get_galaxy_scenario"), &UniverseRenderInfo::GetGalaxyScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("get_star_scenario"), &UniverseRenderInfo::GetStarScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("get_main_scenario"), &UniverseRenderInfo::GetMainScenario);

		ADD_PROPERTY(RID_PROPERTY("galaxy_scenario"), "set_galaxy_scenario", "get_galaxy_scenario");
		ADD_PROPERTY(RID_PROPERTY("star_scenario"), "set_star_scenario", "get_star_scenario");
		ADD_PROPERTY(RID_PROPERTY("main_scenario"), "set_main_scenario", "get_main_scenario");
	}
}