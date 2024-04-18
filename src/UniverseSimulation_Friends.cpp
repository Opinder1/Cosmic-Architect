#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	UniverseSimulation::UUIDVector UniverseSimulation::GetFriends()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.account_info.find_key("friends");
	}

	void UniverseSimulation::InviteFriend(UUID account_id)
	{
		SIM_DEFER_COMMAND(k_commands->invite_friend, account_id);
	}

	void UniverseSimulation::AcceptFreindInvite(UUID account_id)
	{
		SIM_DEFER_COMMAND(k_commands->accept_friend_invite, account_id);
	}

	void UniverseSimulation::RemoveFriend(UUID account_id)
	{
		SIM_DEFER_COMMAND(k_commands->remove_friend, account_id);
	}
}