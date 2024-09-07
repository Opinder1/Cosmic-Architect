#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UUIDVector UniverseSimulation::GetFriends()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.account_info.find_key("friends");
	}

	void UniverseSimulation::InviteFriend(UUID account_id)
	{
		if (DeferCommand(k_commands->invite_friend, account_id))
		{
			return;
		}
	}

	void UniverseSimulation::AcceptFreindInvite(UUID account_id)
	{
		if (DeferCommand(k_commands->accept_friend_invite, account_id))
		{
			return;
		}
	}

	void UniverseSimulation::RemoveFriend(UUID account_id)
	{
		if (DeferCommand(k_commands->remove_friend, account_id))
		{
			return;
		}
	}
}