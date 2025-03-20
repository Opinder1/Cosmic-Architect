#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetGalaxyObjectInfo(const ID& galaxy_object_id)
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

	void UniverseServer::RequestGalaxyObjectInfo(const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::RequestGalaxyObjectInfo>(entity_id))
		{
			return;
		}
	}
}