#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetPartyInfo(UUID party_host_id)
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		auto it = m_info_cache.party_info_map.find(party_host_id);

		if (it != m_info_cache.party_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::CreateParty()
	{
		if (DeferCommand(k_commands->create_party))
		{
			return;
		}
	}

	void UniverseSimulation::InviteToParty(UUID player_id)
	{
		if (DeferCommand(k_commands->invite_to_party, player_id))
		{
			return;
		}
	}

	void UniverseSimulation::AcceptInvite(UUID player_id)
	{
		if (DeferCommand(k_commands->accept_invite, player_id))
		{
			return;
		}
	}

	void UniverseSimulation::KickFromParty()
	{
		if (DeferCommand(k_commands->kick_from_party))
		{
			return;
		}
	}

	void UniverseSimulation::LeaveParty()
	{
		if (DeferCommand(k_commands->leave_party))
		{
			return;
		}
	}

	UUIDVector UniverseSimulation::GetPlayersInParty()
	{
		std::shared_lock lock(m_info_cache.mutex);
		
		godot::Dictionary party = m_info_cache.player_info.find_key("party");

		if (party.is_empty())
		{
			return UUIDVector{};
		}

		return party.find_key("players");
	}

	UUID UniverseSimulation::GetPartyChatChannel()
	{
		std::shared_lock lock(m_info_cache.mutex);

		godot::Dictionary party = m_info_cache.player_info.find_key("party");

		if (party.is_empty())
		{
			return UUID{};
		}

		return party.find_key("chat_channel");
	}
}