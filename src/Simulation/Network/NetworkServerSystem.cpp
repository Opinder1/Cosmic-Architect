#include "NetworkServerSystem.h"
#include "Events.h"
#include "Components.h"

#include "Simulation/Simulation.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/udp_server.hpp>
#include <godot_cpp/classes/dtls_server.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>
#include <godot_cpp/classes/tls_options.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/x509_certificate.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <entt/entity/registry.hpp>

namespace sim
{
	NetworkServerSystem::NetworkServerSystem(Simulation& simulation) :
		System(simulation)
	{
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnTick>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnSimulationRequestStop>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnSimulationStop>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnStartNetworkServer>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnStopNetworkServer>(*this));
	}

	NetworkServerSystem::~NetworkServerSystem()
	{
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnTick>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnSimulationRequestStop>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnSimulationStop>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnStartNetworkServer>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnStopNetworkServer>(*this));
	}

	void NetworkServerSystem::OnTick(const TickEvent& event)
	{
		for (auto [server_entity, server] : Registry().view<ServerComponent>().each())
		{
			server.udp_server->poll();

			if (!Sim().IsStoppingCached())
			{
				while (server.udp_server->is_connection_available())
				{
					auto udp_peer = server.udp_server->take_connection();

					auto dtls_peer = server.dtls_server->take_connection(udp_peer);

					if (dtls_peer->get_status() != godot::PacketPeerDTLS::Status::STATUS_HANDSHAKING)
					{
						continue;
					}

					entt::entity peer_entity = Registry().create();

					Registry().emplace<NewEntityComponent>(peer_entity);
					Registry().emplace<PeerComponent>(peer_entity, udp_peer, dtls_peer);
					Registry().emplace<ChildPeerComponent>(peer_entity, server_entity);
				}
			}
		}
	}

	void NetworkServerSystem::OnSimulationRequestStop(const SimulationRequestStopMessage& event)
	{
		for (auto [server_entity, server] : Registry().view<ServerComponent>().each())
		{
			Registry().emplace<DeletedEntityComponent>(server_entity);
		}
	}

	void NetworkServerSystem::OnSimulationStop(const SimulationStopEvent& event)
	{

	}

	void NetworkServerSystem::OnStartNetworkServer(const StartNetworkServerMessage& event)
	{
		godot::Ref<godot::UDPServer> udp_server;
		godot::Ref<godot::DTLSServer> dtls_server;

		udp_server.instantiate();

		godot::Error error = udp_server->listen(event.port);

		if (error != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR("Server could not listen on port " + godot::String::num_int64(event.port) + " with the error: " + godot::UtilityFunctions::error_string(error));
			return;
		}

		if (!udp_server->is_listening())
		{
			DEBUG_PRINT_ERROR("Failed to start server as it was not listening");
			return;
		}

		dtls_server.instantiate();

		auto options = godot::TLSOptions::server({}, {});

		error = dtls_server->setup(options);

		if (error != godot::Error::OK)
		{
			DEBUG_PRINT_ERROR("Failed to setup dtls server with the error: " + godot::UtilityFunctions::error_string(error));
			return;
		}

		entt::entity server_entity = Registry().create();

		Registry().emplace<NewEntityComponent>(server_entity);
		Registry().emplace<ServerComponent>(server_entity, event.port, udp_server, dtls_server);

		m_servers.emplace(event.port, server_entity);
	}

	void NetworkServerSystem::OnStopNetworkServer(const StopNetworkServerMessage& event)
	{
		auto it = m_servers.find(event.port);

		if (it == m_servers.end())
		{
			DEBUG_PRINT_WARN(godot::vformat("Failed to find the server with port %d", event.port));
			return;
		}

		entt::entity server_entity = it->second;

		for (auto [peer_entity, child_peer] : Registry().view<ChildPeerComponent>().each())
		{
			if (child_peer.server_entity == server_entity)
			{
				Registry().emplace<DeletedEntityComponent>(peer_entity);
			}
		}

		Registry().emplace<DeletedEntityComponent>(server_entity);
	}
}