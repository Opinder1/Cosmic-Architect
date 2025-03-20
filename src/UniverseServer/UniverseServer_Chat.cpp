#include "UniverseServer.h"
#include "UniverseServer_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseServer::GetChannelInfo(const ID& channel_id)
	{
		std::shared_lock lock(m_info_cache.mutex);

		auto it = m_info_cache.chat_channel_info_map.find(channel_id);

		if (it != m_info_cache.chat_channel_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseServer::SendMessageToChannel(const godot::String& message, const ID& channel_id)
	{
		if (DeferCommand<&UniverseServer::SendMessageToChannel>(message, channel_id))
		{
			return;
		}
	}

	void UniverseServer::SendMessageToPlayer(const godot::String& message, const ID& account_id)
	{
		if (DeferCommand<&UniverseServer::SendMessageToPlayer>(message, account_id))
		{
			return;
		}
	}

	godot::Array UniverseServer::GetChatChannelHistory(const ID& channel_id)
	{
		return GetChannelInfo(channel_id).find_key("chat_history");
	}

	godot::Array UniverseServer::GetPrivateChatHistory(const ID& account_id)
	{
		godot::Dictionary private_chats;
		
		{
			std::shared_lock lock(m_info_cache.mutex);
			private_chats = m_info_cache.account_info.find_key("private_chats");
		}

		if (private_chats.is_empty())
		{
			return godot::Array{};
		}

		return private_chats.find_key(account_id);
	}
}