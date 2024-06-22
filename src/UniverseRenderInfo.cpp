#include "UniverseRenderInfo.h"

#include "Util/PropertyMacros.h"

#include <godot_cpp/core/class_db.hpp>

namespace voxel_game
{
	UniverseRenderInfo::UniverseRenderInfo()
	{

	}

	void UniverseRenderInfo::SetScenario(godot::RID scenario)
	{
		m_scenario = scenario;
	}

	godot::RID UniverseRenderInfo::GetScenario() const
	{
		return m_scenario;
	}

	void UniverseRenderInfo::_bind_methods()
	{
		godot::ClassDB::bind_method(godot::D_METHOD("set_scenario", "scenario"), &UniverseRenderInfo::SetScenario);
		godot::ClassDB::bind_method(godot::D_METHOD("get_scenario"), &UniverseRenderInfo::GetScenario);

		ADD_PROPERTY(RID_PROPERTY("scenario"), "set_scenario", "get_scenario");
	}
}