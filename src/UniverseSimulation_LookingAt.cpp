#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UniverseSimulation::UUID UniverseSimulation::GetLookingAtEntity()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("looking_at_entity");
	}

	UniverseSimulation::UUID UniverseSimulation::GetLookingAtVolume()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.player_info.find_key("looking_at_volume");
	}

	godot::Vector4i UniverseSimulation::GetLookingAtBlock()
	{
		return m_read_cache.player_info.find_key("looking_at_block");
	}
}