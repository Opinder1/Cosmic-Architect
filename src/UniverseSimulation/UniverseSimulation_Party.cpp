#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetPartyInfo(const UUID& party_host_id)
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
		if (DeferCommand<&UniverseSimulation::CreateParty>())
		{
			return;
		}
	}

	void UniverseSimulation::InviteToParty(const UUID& player_id)
	{
		if (DeferCommand<&UniverseSimulation::InviteToParty>(player_id))
		{
			return;
		}
	}

	void UniverseSimulation::AcceptInvite(const UUID& player_id)
	{
		if (DeferCommand<&UniverseSimulation::AcceptInvite>(player_id))
		{
			return;
		}
	}

	void UniverseSimulation::KickFromParty()
	{
		if (DeferCommand<&UniverseSimulation::KickFromParty>())
		{
			return;
		}
	}

	void UniverseSimulation::LeaveParty()
	{
		if (DeferCommand<&UniverseSimulation::LeaveParty>())
		{
			return;
		}
	}

	UUIDVector UniverseSimulation::GetPlayersInParty()
	{
		godot::Dictionary party;

		{
			std::shared_lock lock(m_info_cache.mutex);
			party = m_info_cache.player_info.find_key("party");
		}

		if (party.is_empty())
		{
			return UUIDVector{};
		}

		return party.find_key("players");
	}

	UUID UniverseSimulation::GetPartyChatChannel()
	{
		godot::Dictionary party;

		{
			std::shared_lock lock(m_info_cache.mutex);
			party = m_info_cache.player_info.find_key("party");
		}

		if (party.is_empty())
		{
			return UUID{};
		}

		return party.find_key("chat_channel");
	}
}