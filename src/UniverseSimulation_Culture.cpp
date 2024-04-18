#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCultureInfo(UUID culture_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(culture_id);
	}
}