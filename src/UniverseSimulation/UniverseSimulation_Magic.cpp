#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetSpellInfo(const UUID& spell_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
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
		if (DeferCommand<&UniverseSimulation::UseSpell>(spell_index, params))
		{
			return;
		}
	}
}