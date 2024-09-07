#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyObjectInfo(UUID galaxy_object_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.galaxy_object_info_map.find(galaxy_object_id);

		if (it != m_info_cache.galaxy_object_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::RequestGalaxyObjectInfo(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->request_galaxy_object_info, entity_id);
	}
}