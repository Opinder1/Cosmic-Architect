#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyObjectInfo(const UUID& galaxy_object_id)
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

	void UniverseSimulation::RequestGalaxyObjectInfo(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseSimulation::RequestGalaxyObjectInfo>(entity_id))
		{
			return;
		}
	}
}