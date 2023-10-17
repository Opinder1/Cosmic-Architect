#pragma once

#include "Simulation/Message/Message.h"

#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/ip.hpp>

namespace sim
{
	class MessageSender;
	struct NetworkPeer;

	// Send this message to the network messager simulation
	struct StartServerMessage : Message
	{
		explicit StartServerMessage(const MessageSender& sender, uint16_t port) : Message(sender), port(port) {}

		uint16_t port;
	};

	// Send this message to the network messager simulation
	struct StopServerMessage : Message
	{
		explicit StopServerMessage(const MessageSender& sender, uint16_t port) : Message(sender), port(port) {}

		uint16_t port;
	};

	// Send this message to the network messager simulation
	struct StartSubProcessMessage : Message
	{
		bool open_console;
		UUID id;
		uint16_t port;
	};

	// Send this message to the network messager simulation
	struct ConnectToServerMessage : Message
	{
		godot::String host;
		uint16_t port;
	};

	// Send this message to the network messager simulation
	struct DisconnectPeerMessage : Message
	{
		const NetworkPeer* peer;
	};

	// Send this message to the network messager simulation
	struct PostNetworkMessageMessage : Message
	{
		const NetworkPeer* target;
		MessagePtr message;
	};

	// Send this message to the network messager simulation
	struct PostNetworkMessagesMessage : Message
	{
		const NetworkPeer* target;
		MessageQueue message;
	};

	// Send this message to a child peer
	struct RequestShutdownMessage : Message
	{

	};

	// An event that is emitted when a peer connects to the messager
	struct PeerConnectEvent : Event
	{
		const NetworkPeer* peer;
	};

	// An event that is emitted when a peer disconnects to the messager
	struct PeerDisconnectEvent : Event
	{
		const NetworkPeer* peer;
	};
}