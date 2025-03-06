#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	UUID UniverseServer::GetLookingAtEntity()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_entity");
	}

	UUID UniverseServer::GetLookingAtVolume()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_volume");
	}

	godot::Vector4i UniverseServer::GetLookingAtBlock()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("looking_at_block");
	}
}