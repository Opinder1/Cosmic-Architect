#include "NetworkServerSystem.h"
#include "NetworkEvents.h"
#include "NetworkComponents.h"

#include "Simulation/Simulation.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

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
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnSimulationTick>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnStartServer>(*this));
		Sim().Subscribe(cb::Bind<&NetworkServerSystem::OnStopServer>(*this));
	}

	NetworkServerSystem::~NetworkServerSystem()
	{
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnSimulationTick>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnStartServer>(*this));
		Sim().Unsubscribe(cb::Bind<&NetworkServerSystem::OnStopServer>(*this));
	}

	void NetworkServerSystem::OnSimulationTick(const SimulationTickEvent& event)
	{
		for (auto&& [server_entity, server] : Registry().view<ServerComponent>().each())
		{
			server.udp_server->poll();

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
				Registry().emplace<UUIDComponent>(peer_entity, GenerateUUID());
				Registry().emplace<PeerComponent>(peer_entity, udp_peer, dtls_peer);
				Registry().emplace<NewPeerComponent>(peer_entity);
				Registry().emplace<ChildPeerComponent>(peer_entity, server_entity);
			}
		}
	}

	void NetworkServerSystem::OnStartServer(const StartServerMessage& event)
	{
		godot::Ref<godot::UDPServer> udp_server;
		godot::Ref<godot::DTLSServer> dtls_server;

		udp_server.instantiate();

		godot::Error error = udp_server->listen(event.port);

		ERR_FAIL_COND_MSG(error != godot::Error::OK, "Server could not listen on port " + godot::String::num_int64(event.port) + " with the error: " + godot::UtilityFunctions::error_string(error));

		ERR_FAIL_COND_MSG(!udp_server->is_listening(), "Failed to start server as it was not listening");

		dtls_server.instantiate();

		auto options = godot::TLSOptions::server({}, {});

		error = dtls_server->setup(options);

		ERR_FAIL_COND_MSG(error != godot::Error::OK, "Failed to setup dtls server with the error: " + godot::UtilityFunctions::error_string(error));

		entt::entity server_entity = Registry().create();

		Registry().emplace<NewEntityComponent>(server_entity);
		Registry().emplace<ServerComponent>(server_entity, event.port, udp_server, dtls_server);
		Registry().emplace<NewServerComponent>(server_entity);

		m_servers.emplace(event.port, server_entity);
	}

	void NetworkServerSystem::OnStopServer(const StopServerMessage& event)
	{
		/*
		for (auto&& [peer_entity, child_peer] : Registry().view<ChildPeerComponent>().each())
		{
			Registry().emplace<DeletedPeerComponent>(peer_entity);
		}

		if (m_udp_server.is_valid())
		{
			m_dtls_server.unref();
		}

		if (m_dtls_server.is_valid())
		{
			m_udp_server->stop();
		}
		*/
	}
}