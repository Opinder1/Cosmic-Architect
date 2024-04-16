#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetAccountInfo()
	{
		std::shared_lock lock(m_cache_mutex);
		return m_read_cache.account_info;
	}

	void UniverseSimulation::CreateAccount(const godot::String& username, const godot::String& password_hash)
	{
		SIM_DEFER_COMMAND(k_commands->create_account, username, password_hash);
	}

	void UniverseSimulation::AccountLogin(const godot::String& username, const godot::String& password_hash)
	{
		SIM_DEFER_COMMAND(k_commands->account_login, username, password_hash);
	}

	void UniverseSimulation::SavedSessionLogin()
	{
		SIM_DEFER_COMMAND(k_commands->saved_session_login);
	}

	void UniverseSimulation::ClearSavedSession()
	{
		SIM_DEFER_COMMAND(k_commands->clear_saved_session);
	}

	void UniverseSimulation::DeleteAccount()
	{
		SIM_DEFER_COMMAND(k_commands->delete_account);
	}

	void UniverseSimulation::LogoutAccount()
	{
		SIM_DEFER_COMMAND(k_commands->logout_account);
	}
}