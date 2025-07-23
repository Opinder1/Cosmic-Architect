#pragma once

#include "Entity/EntityPoly.h"

#include "Spatial3D/SpatialPoly.h"

#include "Render/RenderInstancer.h"

#include <godot_cpp/classes/x509_certificate.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/dtls_server.hpp>
#include <godot_cpp/classes/udp_server.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>

#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/color.hpp>

#include <robin_hood/robin_hood.h>

#include <array>
#include <vector>

namespace voxel_game
{
	struct CName
	{
		godot::String name;
	};

	struct CRelationship
	{
		entity::Ref parent;
		entity::WRef prev_neighbour;
		entity::WRef next_neighbour;

		size_t children = 0;
		entity::WRef first_child;
	};

	// Entities that are children of a world entity should have this component
	// so that the system knows how to place them in the spatial world
	struct CPosition
	{
		godot::Vector3 position;
		uint8_t scale = 0;
	};

	struct CVelocity
	{
		godot::Vector3 velocity;
	};

	struct CRotation
	{
		godot::Quaternion rotation;
	};

	struct CScale
	{
		godot::Vector3 scale;
	};

	struct CMass
	{
		real_t mass = 0.0;
	};

	struct CBounciness
	{
		real_t bounciness = 0.0;
	};

	struct CRoughness
	{
		real_t roughness = 0.0;
	};

	struct CGravity
	{
		godot::Vector3 force;
	};

	struct CAABB
	{
		godot::AABB aabb;
	};

	struct CBox
	{
		godot::Vector3 size;
	};

	struct CSphere
	{
		real_t radius = 0.0;
	};

	struct CTransform
	{
		godot::Transform3D transform;
		bool modified = false;
	};

	struct CScenario
	{
		godot::RID id;
	};

	struct CInstance
	{
		godot::RID id;
	};

	struct CInstancer
	{
		rendering::Instancer instancer;
	};

	// This entity is a render base which instances will use to define what they render
	struct CBase
	{
		godot::RID id;
	};

	// A tag that denotes that the base is a placeholder cube
	struct CPlaceholderCube {};

	struct CMesh {};

	struct CPointMesh : Nocopy
	{
		godot::PackedVector3Array points;
	};

	struct CMultimesh {};

	struct CParticles {};

	struct CParticlesCollision {};

	struct CLight {};

	struct CReflectionProbe {};

	struct CDecal {};

	struct CVoxelGI {};

	struct CLightmap {};

	struct COccluder {};

	struct CFogVolume {};

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

	// An object that tells a spatial world where to load nodes and at what lods
	struct CLoader
	{
		uint8_t dist_per_lod = 0; // The number of nodes there are until the next lod starts
		uint8_t min_lod = 0; // The minimum lod this camera can see
		uint8_t max_lod = 0; // The maximum lod this camera can see
		uint8_t update_frequency = 0; // The frequency
	};

	// Add this component to a child of a scale marker to signify it represents a region in that world
	struct CRegion
	{
		godot::AABB aabb;
	};

	// An entity which is in a spatial world. It will be given a node its part of and loaded/unloaded with that node
	struct CEntity
	{
		spatial3d::WorldPtr world;
	};

	// A spatial database which has an octree like structure with neighbour pointers and hash maps for each lod. 
	struct CWorld
	{
		spatial3d::WorldPtr world;
	};

	// This entitiy is a universe which has a specialised spatial world
	struct CUniverse
	{

	};

	// This entity is a galaxy
	struct CGalaxy
	{

	};

	// This entity is a star
	struct CStar
	{
		godot::Color color;
	};
}