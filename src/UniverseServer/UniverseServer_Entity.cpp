#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetEntityInfo(const UUID& entity_id)
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

	void UniverseServer::RequestEntityInfo(const UUID& entity_id)
	{
		if (DeferCommand<&UniverseServer::RequestEntityInfo>(entity_id))
		{
			return;
		}
	}
}