#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetSpellInfo(UUID spell_id)
	{
		std::shared_lock lock(m_cache_mutex);
		
		auto it = m_info_cache.spell_info_map.find(spell_id);

		if (it != m_info_cache.spell_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::UseSpell(uint64_t spell_index, const godot::Dictionary& params)
	{
		SIM_DEFER_COMMAND(k_commands->use_spell, spell_index, params);
	}
}