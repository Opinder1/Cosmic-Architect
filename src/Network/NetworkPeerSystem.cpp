#include "NetworkPeerSystem.h"
#include "NetworkEvents.h"
#include "NetworkComponents.h"

#include "Simulation/Simulation.h"
#include "Simulation/SimulationServer.h"
#include "Simulation/Events.h"
#include "Simulation/Components.h"

#include "Simulation/Stream/ByteArrayStream.h"
#include "Simulation/Stream/StreamHelper.h"

#include "Simulation/Message/MessageRegistry.h"

#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

#include <entt/entity/registry.hpp>

namespace sim
{
	enum class NetworkMessageType : uint8_t
	{
		Message,
	};

	NetworkPeerSystem::NetworkPeerSystem(Simulation& simulation) :
		System(simulation)
	{
		Sim().Subscribe(cb::Bind<&NetworkPeerSystem::OnSimulationTick>(*this));
	}

	NetworkPeerSystem::~NetworkPeerSystem()
	{
		Sim().Unsubscribe(cb::Bind<&NetworkPeerSystem::OnSimulationTick>(*this));
	}

	void NetworkPeerSystem::OnSimulationTick(const SimulationTickEvent& event)
	{
		for (auto&& [peer_entity, peer] : Registry().view<PeerComponent>().each())
		{
			peer.dtls_peer->poll();

			size_t packets = peer.dtls_peer->get_available_packet_count();

			for (size_t packet = 0; packet < packets; packet++)
			{
				godot::PackedByteArray packet_data = peer.dtls_peer->get_packet();

				godot::Error error = peer.dtls_peer->get_packet_error();

				ERR_CONTINUE_MSG(error != godot::Error::OK, godot::String("The packet had an error: ") + godot::UtilityFunctions::error_string(error));

				ByteArrayStream stream{ packet_data };

				NetworkMessageType type;
				if (!StreamHelper::ReadEnum(stream, type))
				{
					continue;
				}

				switch (type)
				{
				case NetworkMessageType::Message:
					LoadMessage(stream);
					break;

				default:
					ERR_PRINT("Invalid network message type");
				}
			}
		}
	}

	bool NetworkPeerSystem::LoadMessage(ByteStream& stream)
	{
		// Get the message and the target to send to
		UUID target;
		MessagePtr message = MessageRegistry::GetSingleton()->UnpackMessage(stream, target);

		ERR_FAIL_COND_V_MSG(!message, false, "No message could be unpacked from the packet data");

		Sim().PostMessageToOther(target, message);

		return true;
	}
}