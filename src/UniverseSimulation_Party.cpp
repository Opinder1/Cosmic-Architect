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
		SIM_DEFER_COMMAND(k_commands->create_party);
	}

	void UniverseSimulation::InviteToParty(UUID player_id)
	{
		SIM_DEFER_COMMAND(k_commands->invite_to_party, player_id);
	}

	void UniverseSimulation::AcceptInvite(UUID player_id)
	{
		SIM_DEFER_COMMAND(k_commands->accept_invite, player_id);
	}

	void UniverseSimulation::KickFromParty()
	{
		SIM_DEFER_COMMAND(k_commands->kick_from_party);
	}

	void UniverseSimulation::LeaveParty()
	{
		SIM_DEFER_COMMAND(k_commands->leave_party);
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