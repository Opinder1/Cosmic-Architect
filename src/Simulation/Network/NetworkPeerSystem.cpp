#include "NetworkPeerSystem.h"
#include "Events.h"
#include "Components.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

#include "Simulation/Message/MessageRegistry.h"

#include "Simulation/Stream/ByteArrayStream.h"
#include "Simulation/Stream/StreamHelper.h"

#include "Util/Debug.h"

#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

namespace sim
{
	enum class NetworkMessageType : uint8_t
	{
		Message,
	};

	void NetworkPeerSystem::OnInitialize(Simulation& simulation)
	{
		simulation.dispatcher.Subscribe(cb::BindParam<&NetworkPeerSystem::OnSimulationTick>(simulation));
	}

	void NetworkPeerSystem::OnShutdown(Simulation& simulation)
	{
		simulation.dispatcher.Unsubscribe(cb::BindParam<&NetworkPeerSystem::OnSimulationTick>(simulation));
	}

	void NetworkPeerSystem::OnSimulationTick(Simulation& simulation, const SimulationTickEvent& event)
	{
		for (auto&& [peer_entity, peer] : simulation.registry.view<PeerComponent>().each())
		{
			peer.dtls_peer->poll();

			size_t packets = peer.dtls_peer->get_available_packet_count();

			for (size_t packet = 0; packet < packets; packet++)
			{
				godot::PackedByteArray packet_data = peer.dtls_peer->get_packet();

				godot::Error error = peer.dtls_peer->get_packet_error();

				if (error != godot::Error::OK)
				{
					DEBUG_PRINT_ERROR(godot::String("The packet had an error: ") + godot::UtilityFunctions::error_string(error));
					continue;
				}

				ByteArrayStream stream{ packet_data };

				NetworkMessageType type;
				if (!StreamHelper::ReadEnum(stream, type))
				{
					continue;
				}

				switch (type)
				{
				case NetworkMessageType::Message:
					LoadMessage(simulation, stream);
					break;

				default:
					DEBUG_PRINT_ERROR("Invalid network message type");
					break;
				}
			}
		}
	}

	bool NetworkPeerSystem::LoadMessage(Simulation& simulation, ByteStream& stream)
	{
		// Get the message and the target to send to
		UUID target;
		MessagePtr message = simulation.server.UnpackMessage(stream, target);

		if (message == nullptr)
		{
			DEBUG_PRINT_ERROR("No message could be unpacked from the packet data");
			return false;
		}

		simulation.messager.PostMessageToOther(target, message);

		return true;
	}
}