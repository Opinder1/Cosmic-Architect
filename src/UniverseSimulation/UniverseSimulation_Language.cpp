#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetLanguageInfo(const UUID& language_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.language_info_map.find(language_id);

		if (it != m_info_cache.language_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::GetLanguageTranslation(const UUID& language_id, const godot::StringName& string)
	{
		if (DeferCommand<&UniverseSimulation::GetLanguageTranslation>(language_id, string))
		{
			return;
		}
	}

	void UniverseSimulation::GetLanguageString(const UUID& language_id, uint64_t string_id)
	{
		if (DeferCommand<&UniverseSimulation::GetLanguageString>(language_id, string_id))
		{
			return;
		}
	}
}