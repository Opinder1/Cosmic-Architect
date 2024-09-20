#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UUID UniverseSimulation::GetLookingAtEntity()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_entity");
	}

	UUID UniverseSimulation::GetLookingAtVolume()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_volume");
	}

	godot::Vector4i UniverseSimulation::GetLookingAtBlock()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_block");
	}
}