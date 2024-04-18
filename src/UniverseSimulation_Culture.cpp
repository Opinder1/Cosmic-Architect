#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetCultureInfo(UUID culture_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_read_cache.culture_info_map.find(culture_id);

		if (it != m_read_cache.culture_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}
}