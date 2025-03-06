#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetFactionInfo(const UUID& faction_id)
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

	UUIDVector UniverseServer::GetJoinedFactions()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("joined_factions");
	}

	void UniverseServer::JoinFaction(const UUID& faction_id, const godot::Dictionary& request_info)
	{
		if (DeferCommand<&UniverseServer::JoinFaction>(faction_id, request_info))
		{
			return;
		}
	}

	void UniverseServer::LeaveFaction(const UUID& faction_id)
	{
		if (DeferCommand<&UniverseServer::LeaveFaction>(faction_id))
		{
			return;
		}
	}

	void UniverseServer::InviteEntityToFaction(const UUID& faction_id, const UUID& entity_id)
	{
		if (DeferCommand<&UniverseServer::InviteEntityToFaction>(faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseServer::KickEntityFromFaction(const UUID& faction_id, const UUID& entity_id)
	{
		if (DeferCommand<&UniverseServer::KickEntityFromFaction>(faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseServer::AddChildFaction(const UUID& parent_faction_id, const UUID& child_faction_id)
	{
		if (DeferCommand<&UniverseServer::AddChildFaction>(parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseServer::RemoveChildFaction(const UUID& faction_id)
	{
		if (DeferCommand<&UniverseServer::RemoveChildFaction>(faction_id))
		{
			return;
		}
	}

	void UniverseServer::InviteChildFaction(const UUID& parent_faction_id, const UUID& child_faction_id)
	{
		if (DeferCommand<&UniverseServer::InviteChildFaction>(parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseServer::KickChildFaction(const UUID& faction_id)
	{
		if (DeferCommand<&UniverseServer::KickChildFaction>(faction_id))
		{
			return;
		}
	}
}