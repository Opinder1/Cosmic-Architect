#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetChannelInfo(UUID channel_id)
	{
		std::shared_lock lock(m_cache_mutex);

		auto it = m_read_cache.chat_channel_info_map.find(channel_id);

		if (it != m_read_cache.chat_channel_info_map.end())
		{
			return it->second;
		}
		else
		{
			return godot::Dictionary{};
		}
	}

	void UniverseSimulation::SendMessageToChannel(const godot::String& message, UUID channel_id)
	{
		SIM_DEFER_COMMAND(k_commands->send_message_to_channel, message, channel_id);
	}

	void UniverseSimulation::SendMessageToPlayer(const godot::String& message, UUID account_id)
	{
		SIM_DEFER_COMMAND(k_commands->send_message_to_player, message, account_id);
	}

	godot::Array UniverseSimulation::GetChatChannelHistory(UUID channel_id)
	{
		return GetChannelInfo(channel_id).find_key("chat_history");
	}

	godot::Array UniverseSimulation::GetPrivateChatHistory(UUID account_id)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Dictionary private_chats = m_read_cache.account_info.find_key("private_chats");

		if (private_chats.is_empty())
		{
			return godot::Array{};
		}

		return private_chats.find_key(account_id);
	}
}