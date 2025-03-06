#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetAccountInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.account_info;
	}

	void UniverseServer::CreateAccount(const godot::String& username, const godot::String& password_hash)
	{
		if (DeferCommand<&UniverseServer::CreateAccount>(username, password_hash))
		{
			return;
		}
	}

	void UniverseServer::AccountLogin(const godot::String& username, const godot::String& password_hash)
	{
		if (DeferCommand<&UniverseServer::AccountLogin>(username, password_hash))
		{
			return;
		}
	}

	void UniverseServer::SavedSessionLogin()
	{
		if (DeferCommand<&UniverseServer::SavedSessionLogin>())
		{
			return;
		}
	}

	void UniverseServer::ClearSavedSession()
	{
		if (DeferCommand<&UniverseServer::ClearSavedSession>())
		{
			return;
		}
	}

	void UniverseServer::DeleteAccount()
	{
		if (DeferCommand<&UniverseServer::DeleteAccount>())
		{
			return;
		}
	}

	void UniverseServer::LogoutAccount()
	{
		if (DeferCommand<&UniverseServer::LogoutAccount>())
		{
			return;
		}
	}
}