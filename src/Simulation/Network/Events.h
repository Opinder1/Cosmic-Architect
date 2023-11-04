#pragma once

#include "Simulation/Message/Message.h"

#include "Simulation/UUID.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/ip.hpp>

namespace sim
{
	class MessageSender;
	struct NetworkPeer;

	// Send this message to the network messager simulation
	struct StartNetworkServerMessage : Message
	{
		explicit StartNetworkServerMessage(const MessageSender& sender, uint16_t port) : Message(sender), port(port) {}

		uint16_t port;
	};

	// Send this message to the network messager simulation
	struct StopNetworkServerMessage : Message
	{
		explicit StopNetworkServerMessage(const MessageSender& sender, uint16_t port) : Message(sender), port(port) {}

		uint16_t port;
	};

	// Send messages to a remote simulation
	struct MessageRemoteSimulationMessage : Message
	{
		explicit MessageRemoteSimulationMessage(const MessageSender& sender, UUID remote_id) : Message(sender), remote_id(remote_id) {}

		UUID remote_id;

		MessageQueue messages;
	};
}