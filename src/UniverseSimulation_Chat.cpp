#include "UniverseSimulation.h"

namespace voxel_game
{
	godot::Dictionary UniverseSimulation::GetChannelInfo(UUID channel_id)
	{
		return {};
	}

	void UniverseSimulation::SendMessageToChannel(const godot::String& message, UUID channel_id)
	{

	}

	void UniverseSimulation::SendMessageToPlayer(const godot::String& message, UUID account_id)
	{

	}

	godot::Array UniverseSimulation::GetChatChannelHistory(UUID channel_id)
	{
		return {};
	}

	godot::Array UniverseSimulation::GetPrivateChatHistory(UUID account_id)
	{
		return {};
	}
}