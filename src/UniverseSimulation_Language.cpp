#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetLanguageInfo(UUID language_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(language_id);
	}

	void UniverseSimulation::GetLanguageTranslation(UUID language_id, const godot::StringName& string)
	{
		SIM_DEFER_COMMAND(k_commands->get_language_translation, language_id, string);
	}

	void UniverseSimulation::GetLanguageString(UUID language_id, uint64_t string_id)
	{
		SIM_DEFER_COMMAND(k_commands->get_language_string, language_id, string_id);
	}
}