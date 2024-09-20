#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetChannelInfo(const UUID& channel_id)
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

	void UniverseSimulation::SendMessageToChannel(const godot::String& message, const UUID& channel_id)
	{
		if (DeferCommand(k_commands->send_message_to_channel, message, channel_id))
		{
			return;
		}
	}

	void UniverseSimulation::SendMessageToPlayer(const godot::String& message, const UUID& account_id)
	{
		if (DeferCommand(k_commands->send_message_to_player, message, account_id))
		{
			return;
		}
	}

	godot::Array UniverseSimulation::GetChatChannelHistory(const UUID& channel_id)
	{
		return GetChannelInfo(channel_id).find_key("chat_history");
	}

	godot::Array UniverseSimulation::GetPrivateChatHistory(const UUID& account_id)
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