#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

#include "Util/Debug.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetFragmentInfo(const ID& fragment_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.fragment_info_map.find(fragment_id);

		if (it != m_info_cache.fragment_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	ID UniverseServer::GetCurrentFragment()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("current_fragment");
	}

	void UniverseServer::EnterFragment(const ID& fragment_id, const godot::Dictionary& method)
	{
		if (DeferCommand<&UniverseServer::EnterFragment>(fragment_id, method))
		{
			return;
		}
	}
}