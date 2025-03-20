#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetPartyInfo(const ID& party_host_id)
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

	void UniverseServer::CreateParty()
	{
		if (DeferCommand<&UniverseServer::CreateParty>())
		{
			return;
		}
	}

	void UniverseServer::InviteToParty(const ID& player_id)
	{
		if (DeferCommand<&UniverseServer::InviteToParty>(player_id))
		{
			return;
		}
	}

	void UniverseServer::AcceptInvite(const ID& player_id)
	{
		if (DeferCommand<&UniverseServer::AcceptInvite>(player_id))
		{
			return;
		}
	}

	void UniverseServer::KickFromParty()
	{
		if (DeferCommand<&UniverseServer::KickFromParty>())
		{
			return;
		}
	}

	void UniverseServer::LeaveParty()
	{
		if (DeferCommand<&UniverseServer::LeaveParty>())
		{
			return;
		}
	}

	IDVector UniverseServer::GetPlayersInParty()
	{
		godot::Dictionary party;

		{
			std::shared_lock lock(m_info_cache.mutex);
			party = m_info_cache.player_info.find_key("party");
		}

		if (party.is_empty())
		{
			return IDVector{};
		}

		return party.find_key("players");
	}

	ID UniverseServer::GetPartyChatChannel()
	{
		godot::Dictionary party;

		{
			std::shared_lock lock(m_info_cache.mutex);
			party = m_info_cache.player_info.find_key("party");
		}

		if (party.is_empty())
		{
			return ID{};
		}

		return party.find_key("chat_channel");
	}
}