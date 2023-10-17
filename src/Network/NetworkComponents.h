#pragma once

#include <godot_cpp/classes/ref.hpp>

#include <entt/entity/entity.hpp>

namespace godot
{
	class UDPServer;
	class DTLSServer;
	class PacketPeerUDP;
	class PacketPeerDTLS;
	class TLSOptions;
}

namespace sim
{
	struct ServerComponent
	{
		uint16_t port;

		godot::Ref<godot::UDPServer> udp_server;
		godot::Ref<godot::DTLSServer> dtls_server;

		std::vector<entt::entity> child_peers;
	};

	struct NewServerComponent {};

	struct DeletedServerComponent {};

	// This component makes this entity a peer
	struct PeerComponent
	{
		godot::Ref<godot::PacketPeerUDP> udp_peer;
		godot::Ref<godot::PacketPeerDTLS> dtls_peer;

		std::vector<entt::entity> child_remote_simulations;
	};

	struct NewPeerComponent {};

	struct DeletedPeerComponent {};

	struct ChildPeerComponent
	{
		entt::entity server_entity;
	};

	struct ParentPeerComponent {};
	
	struct SubProcessComponent
	{
		int64_t sub_process_id;
	};

	// This component makes this entity a remote simulation
	struct RemoteSimulationComponent
	{
		entt::entity owning_peer;

		MessageQueue queue;
	};

	struct NewRemoteSimulationComponent {};

	struct DeletedRemoteSimulationComponent {};
}