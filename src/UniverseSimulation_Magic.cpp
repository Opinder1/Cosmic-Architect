#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetSpellInfo(UUID spell_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(spell_id);
	}

	void UniverseSimulation::UseSpell(uint64_t spell_index, const godot::Dictionary& params)
	{
		SIM_DEFER_COMMAND(k_commands->use_spell, spell_index, params);
	}
}