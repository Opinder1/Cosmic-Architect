#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetFactionInfo(UUID faction_id)
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

	UUIDVector UniverseSimulation::GetJoinedFactions()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.player_info.find_key("joined_factions");
	}

	void UniverseSimulation::JoinFaction(UUID faction_id, const godot::Dictionary& request_info)
	{
		if (DeferCommand(k_commands->join_faction, faction_id, request_info))
		{
			return;
		}
	}

	void UniverseSimulation::LeaveFaction(UUID faction_id)
	{
		if (DeferCommand(k_commands->leave_faction, faction_id))
		{
			return;
		}
	}

	void UniverseSimulation::InviteEntityToFaction(UUID faction_id, UUID entity_id)
	{
		if (DeferCommand(k_commands->invite_entity_to_faction, faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::KickEntityFromFaction(UUID faction_id, UUID entity_id)
	{
		if (DeferCommand(k_commands->kick_entity_from_faction, faction_id, entity_id))
		{
			return;
		}
	}

	void UniverseSimulation::AddChildFaction(UUID parent_faction_id, UUID child_faction_id)
	{
		if (DeferCommand(k_commands->add_child_faction, parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseSimulation::RemoveChildFaction(UUID faction_id)
	{
		if (DeferCommand(k_commands->remove_child_faction, faction_id))
		{
			return;
		}
	}

	void UniverseSimulation::InviteChildFaction(UUID parent_faction_id, UUID child_faction_id)
	{
		if (DeferCommand(k_commands->invite_child_faction, parent_faction_id, child_faction_id))
		{
			return;
		}
	}

	void UniverseSimulation::KickChildFaction(UUID faction_id)
	{
		if (DeferCommand(k_commands->kick_child_faction, faction_id))
		{
			return;
		}
	}
}