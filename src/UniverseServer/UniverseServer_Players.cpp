#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetPlayerInfo(const ID& player_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.player_info_map.find(player_id);

		if (it != m_info_cache.player_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}
}