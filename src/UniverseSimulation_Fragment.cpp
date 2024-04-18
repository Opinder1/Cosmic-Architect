#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Util/Debug.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetFragmentInfo(UUID fragment_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(fragment_id);
	}

	UniverseSimulation::UUID UniverseSimulation::GetCurrentFragment()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("current_fragment");
	}

	void UniverseSimulation::EnterFragment(UUID fragment_id, const godot::Dictionary& method)
	{
		SIM_DEFER_COMMAND(k_commands->enter_fragment, fragment_id, method);
	}
}