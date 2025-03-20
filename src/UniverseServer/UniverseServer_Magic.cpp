#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetSpellInfo(const ID& spell_id)
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

	void UniverseServer::UseSpell(uint64_t spell_index, const godot::Dictionary& params)
	{
		if (DeferCommand<&UniverseServer::UseSpell>(spell_index, params))
		{
			return;
		}
	}
}