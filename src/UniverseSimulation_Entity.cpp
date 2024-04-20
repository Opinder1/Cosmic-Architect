#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetEntityInfo(UUID entity_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_info_cache.entity_info_map.find(entity_id);

		if (it != m_info_cache.entity_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::RequestEntityInfo(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->request_entity_info, entity_id);
	}
}