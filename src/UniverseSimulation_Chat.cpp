#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetChannelInfo(UUID channel_id)
	{
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(channel_id);
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
		std::shared_lock lock(m_cache_mutex);
		return GetCacheEntry(channel_id).find_key("chat_history");
	}

	godot::Array UniverseSimulation::GetPrivateChatHistory(UUID account_id)
	{
		std::shared_lock lock(m_cache_mutex);

		godot::Variant relationship = m_read_cache.account_info.find_key(account_id);

		if (relationship.get_type() == godot::Variant::DICTIONARY)
		{
			return relationship.operator godot::Dictionary().find_key("chat_history");
		}

		return godot::Array{};
	}
}