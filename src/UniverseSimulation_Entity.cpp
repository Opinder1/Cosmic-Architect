#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetEntityInfo(UUID entity_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

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
		if (DeferCommand(k_commands->request_entity_info, entity_id))
		{
			return;
		}
	}
}