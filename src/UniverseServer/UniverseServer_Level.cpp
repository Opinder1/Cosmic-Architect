#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	uint16_t UniverseServer::GetLevel()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("level");
	}

	uint64_t UniverseServer::GetExperience()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("experience");
	}

	void UniverseServer::CompleteLevelUp(const godot::Dictionary& levelup_choices)
	{
		if (DeferCommand<&UniverseServer::CompleteLevelUp>(levelup_choices))
		{
			return;
		}
	}
}