#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	ID UniverseServer::GetGlobalPlayerFaction()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.galaxy_info.find_key("player_faction");
	}

	ID UniverseServer::GetPlayerFaction()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("faction");
	}

	void UniverseServer::RequestJoinPlayerFaction(const ID& faction_id, const godot::String& message)
	{
		if (DeferCommand<&UniverseServer::RequestJoinPlayerFaction>(faction_id, message))
		{
			return;
		}
	}
}