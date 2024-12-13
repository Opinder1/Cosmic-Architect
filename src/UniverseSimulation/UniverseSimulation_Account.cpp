#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetAccountInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.account_info;
	}

	void UniverseSimulation::CreateAccount(const godot::String& username, const godot::String& password_hash)
	{
		if (DeferCommand<&UniverseSimulation::CreateAccount>(username, password_hash))
		{
			return;
		}
	}

	void UniverseSimulation::AccountLogin(const godot::String& username, const godot::String& password_hash)
	{
		if (DeferCommand<&UniverseSimulation::AccountLogin>(username, password_hash))
		{
			return;
		}
	}

	void UniverseSimulation::SavedSessionLogin()
	{
		if (DeferCommand<&UniverseSimulation::SavedSessionLogin>())
		{
			return;
		}
	}

	void UniverseSimulation::ClearSavedSession()
	{
		if (DeferCommand<&UniverseSimulation::ClearSavedSession>())
		{
			return;
		}
	}

	void UniverseSimulation::DeleteAccount()
	{
		if (DeferCommand<&UniverseSimulation::DeleteAccount>())
		{
			return;
		}
	}

	void UniverseSimulation::LogoutAccount()
	{
		if (DeferCommand<&UniverseSimulation::LogoutAccount>())
		{
			return;
		}
	}
}