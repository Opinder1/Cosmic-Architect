#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UUIDVector UniverseSimulation::GetFriends()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.account_info.find_key("friends");
	}

	void UniverseSimulation::InviteFriend(const UUID& account_id)
	{
		if (DeferCommand<&UniverseSimulation::InviteFriend>(account_id))
		{
			return;
		}
	}

	void UniverseSimulation::AcceptFreindInvite(const UUID& account_id)
	{
		if (DeferCommand<&UniverseSimulation::AcceptFreindInvite>(account_id))
		{
			return;
		}
	}

	void UniverseSimulation::RemoveFriend(const UUID& account_id)
	{
		if (DeferCommand<&UniverseSimulation::RemoveFriend>(account_id))
		{
			return;
		}
	}
}