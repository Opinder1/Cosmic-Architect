#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	IDVector UniverseServer::GetFriends()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.account_info.find_key("friends");
	}

	void UniverseServer::InviteFriend(const ID& account_id)
	{
		if (DeferCommand<&UniverseServer::InviteFriend>(account_id))
		{
			return;
		}
	}

	void UniverseServer::AcceptFreindInvite(const ID& account_id)
	{
		if (DeferCommand<&UniverseServer::AcceptFreindInvite>(account_id))
		{
			return;
		}
	}

	void UniverseServer::RemoveFriend(const ID& account_id)
	{
		if (DeferCommand<&UniverseServer::RemoveFriend>(account_id))
		{
			return;
		}
	}
}