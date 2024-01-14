#include "NetworkServerSystem.h"
#include "Events.h"
#include "Components.h"
#include "Globals.h"

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
	void NetworkServerSystem::OnInitialize(Simulation& simulation)
	{
		simulation.messager.Subscribe(cb::BindParam<&NetworkServerSystem::OnSimulationTick>(simulation));
		simulation.messager.Subscribe(cb::BindParam<&NetworkServerSystem::OnSimulationRequestStop>(simulation));
		simulation.messager.Subscribe(cb::BindParam<&NetworkServerSystem::OnSimulationStop>(simulation));
		simulation.messager.Subscribe(cb::BindParam<&NetworkServerSystem::OnStartNetworkServer>(simulation));
		simulation.messager.Subscribe(cb::BindParam<&NetworkServerSystem::OnStopNetworkServer>(simulation));
	}

	void NetworkServerSystem::OnShutdown(Simulation& simulation)
	{
		simulation.messager.Unsubscribe(cb::BindParam<&NetworkServerSystem::OnStopNetworkServer>(simulation));
		simulation.messager.Unsubscribe(cb::BindParam<&NetworkServerSystem::OnStartNetworkServer>(simulation));
		simulation.messager.Unsubscribe(cb::BindParam<&NetworkServerSystem::OnSimulationStop>(simulation));
		simulation.messager.Unsubscribe(cb::BindParam<&NetworkServerSystem::OnSimulationRequestStop>(simulation));
		simulation.messager.Unsubscribe(cb::BindParam<&NetworkServerSystem::OnSimulationTick>(simulation));
	}

	void NetworkServerSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		bool sim_stopping = simulation.messager.IsStopping();

		for (auto [server_entity, server] : simulation.registry.view<ServerComponent>().each())
		{
			server.udp_server->poll();

			if (!sim_stopping)
			{
				while (server.udp_server->is_connection_available())
				{
					auto udp_peer = server.udp_server->take_connection();

					auto dtls_peer = server.dtls_server->take_connection(udp_peer);

					if (dtls_peer->get_status() != godot::PacketPeerDTLS::Status::STATUS_HANDSHAKING)
					{
						continue;
					}

					entt::entity peer_entity = simulation.registry.create();

					simulation.registry.emplace<NewComponent>(peer_entity);
					simulation.registry.emplace<PeerComponent>(peer_entity, udp_peer, dtls_peer);
					simulation.registry.emplace<ChildPeerComponent>(peer_entity, server_entity);
				}
			}
		}
	}

	void NetworkServerSystem::OnSimulationRequestStop(Simulation& simulation, const SimulationRequestStopMessage& event)
	{
		for (auto [server_entity, server] : simulation.registry.view<ServerComponent>().each())
		{
			simulation.registry.emplace<DeletedComponent>(server_entity);
		}
	}

	void NetworkServerSystem::OnSimulationStop(Simulation& simulation, const SimulationStopEvent& event)
	{

	}

	void NetworkServerSystem::OnStartNetworkServer(Simulation& simulation, const StartNetworkServerMessage& event)
	{
		auto& server_entities = simulation.globals.get<ServersGlobal>().server_entities;

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

		entt::entity server_entity = simulation.registry.create();

		simulation.registry.emplace<NewComponent>(server_entity);
		simulation.registry.emplace<ServerComponent>(server_entity, event.port, udp_server, dtls_server);

		server_entities.emplace(event.port, server_entity);
	}

	void NetworkServerSystem::OnStopNetworkServer(Simulation& simulation, const StopNetworkServerMessage& event)
	{
		auto& server_entities = simulation.globals.get<ServersGlobal>().server_entities;

		auto it = server_entities.find(event.port);

		if (it == server_entities.end())
		{
			DEBUG_PRINT_WARN(godot::vformat("Failed to find the server with port %d", event.port));
			return;
		}

		entt::entity server_entity = it->second;

		for (auto [peer_entity, child_peer] : simulation.registry.view<ChildPeerComponent>().each())
		{
			if (child_peer.server_entity == server_entity)
			{
				simulation.registry.emplace<DeletedComponent>(peer_entity);
			}
		}

		simulation.registry.emplace<DeletedComponent>(server_entity);
	}
}