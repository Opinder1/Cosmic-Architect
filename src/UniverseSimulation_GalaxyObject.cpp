#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetGalaxyObjectInfo(UUID galaxy_object_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(galaxy_object_id);
	}

	void UniverseSimulation::RequestGalaxyObjectInfo(UUID entity_id)
	{
		SIM_DEFER_COMMAND(k_commands->request_galaxy_object_info, entity_id);
	}
}