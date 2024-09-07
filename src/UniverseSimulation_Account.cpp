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
		if (DeferCommand(k_commands->create_account, username, password_hash))
		{
			return;
		}
	}

	void UniverseSimulation::AccountLogin(const godot::String& username, const godot::String& password_hash)
	{
		if (DeferCommand(k_commands->account_login, username, password_hash))
		{
			return;
		}
	}

	void UniverseSimulation::SavedSessionLogin()
	{
		if (DeferCommand(k_commands->saved_session_login))
		{
			return;
		}
	}

	void UniverseSimulation::ClearSavedSession()
	{
		if (DeferCommand(k_commands->clear_saved_session))
		{
			return;
		}
	}

	void UniverseSimulation::DeleteAccount()
	{
		if (DeferCommand(k_commands->delete_account))
		{
			return;
		}
	}

	void UniverseSimulation::LogoutAccount()
	{
		if (DeferCommand(k_commands->logout_account))
		{
			return;
		}
	}
}