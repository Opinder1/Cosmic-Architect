#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetFactionInfo(const ID& faction_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.faction_info_map.find(faction_id);

		if (it != m_info_cache.faction_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	IDVector UniverseServer::GetJoinedFactions()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("joined_factions");
	}

	void UniverseServer::JoinFaction(const ID& faction_id, const godot::Dictionary& request_info)
	{
		if (DeferCommand<&UniverseServer::JoinFaction>(faction_id, request_info))
		{
			return;
		}
	}

	void UniverseServer::LeaveFaction(const ID& faction_id)
	{
		if (DeferCommand<&UniverseServer::LeaveFaction>(faction_id))
		{
			return;
		}
	}

	void UniverseServer::InviteEntityToFaction(const ID& faction_id, const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::InviteEntityToFaction>(faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseServer::KickEntityFromFaction(const ID& faction_id, const ID& entity_id)
	{
		if (DeferCommand<&UniverseServer::KickEntityFromFaction>(faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseServer::AddChildFaction(const ID& parent_faction_id, const ID& child_faction_id)
	{
		if (DeferCommand<&UniverseServer::AddChildFaction>(parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseServer::RemoveChildFaction(const ID& faction_id)
	{
		if (DeferCommand<&UniverseServer::RemoveChildFaction>(faction_id))
		{
			return;
		}
	}

	void UniverseServer::InviteChildFaction(const ID& parent_faction_id, const ID& child_faction_id)
	{
		if (DeferCommand<&UniverseServer::InviteChildFaction>(parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseServer::KickChildFaction(const ID& faction_id)
	{
		if (DeferCommand<&UniverseServer::KickChildFaction>(faction_id))
		{
			return;
		}
	}
}