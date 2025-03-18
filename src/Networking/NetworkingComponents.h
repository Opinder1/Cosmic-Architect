#pragma once

#include <godot_cpp/classes/x509_certificate.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/dtls_server.hpp>
#include <godot_cpp/classes/udp_server.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>

#include <flecs/flecs.h>

namespace voxel_game::network
{
	struct Components
	{
		Components(flecs::world& world);
	};

	struct CCertificate
	{
		godot::Ref<godot::X509Certificate> certificate;
	};

	struct CServer
	{
		godot::Ref<godot::UDPServer> server_udp;
		godot::Ref<godot::DTLSServer> server_dtls;
	};

	struct CPeer
	{
		godot::Ref<godot::CryptoKey> key;
		godot::Ref<godot::PacketPeerUDP> peer_udp;
		godot::Ref<godot::PacketPeerDTLS> peer_dtls;
	};
}