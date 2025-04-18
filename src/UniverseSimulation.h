#pragma once

#include "Spatial3D/SpatialPoly.h"
#include "Spatial3D/SpatialWorld.h"

#include "Entity/EntityPoly.h"

#include <godot_cpp/classes/x509_certificate.hpp>
#include <godot_cpp/classes/crypto_key.hpp>
#include <godot_cpp/classes/dtls_server.hpp>
#include <godot_cpp/classes/udp_server.hpp>
#include <godot_cpp/classes/packet_peer_udp.hpp>
#include <godot_cpp/classes/packet_peer_dtls.hpp>

namespace voxel_game
{
	struct Simulation;

	struct TaskData
	{
		Simulation& simulation;
		void(*callback)(Simulation&, size_t);
		size_t count;
	};

	struct SpatialTypeData
	{
		spatial3d::WorldType world_type;
		spatial3d::ScaleType scale_type;
		spatial3d::NodeType node_type;

		size_t max_world_scale = spatial3d::k_max_world_scale;

		std::vector<spatial3d::WorldPtr> worlds;
		std::vector<spatial3d::ScalePtr> scales;
	};

	struct Simulation
	{
		// Simulation
		uint8_t processor_count = 0;
		bool high_priority = true;
		bool thread_mode = false;
		uint8_t worker_count = 1;

		uint64_t frame_index = 0;
		Clock::time_point frame_start_time;

		// Entity
		entity::Factory entity_factory;

		std::vector<entity::Ref> entities;

		// Spatial
		std::vector<spatial3d::WorldPtr> spatial_worlds;
		std::vector<spatial3d::ScalePtr> spatial_scales;

		SpatialTypeData universe_type;
		SpatialTypeData galaxy_type;
		SpatialTypeData star_system_type;
		SpatialTypeData planet_type;
		SpatialTypeData space_station_type;
		SpatialTypeData space_ship_type;
		SpatialTypeData vehicle_type;

		// Universe
		entity::Ref universe;

		// Galaxy
		std::vector<entity::Ref> galaxies;

		// StarSystem
		std::vector<entity::Ref> star_systems;

		// Network
		godot::Ref<godot::X509Certificate> server_certificate;
		godot::Ref<godot::UDPServer> server_udp;
		godot::Ref<godot::DTLSServer> server_dtls;
	};

	void SimulationDoTasks(Simulation& simulation, TaskData& task_data);

	void SimulationDoMultitasks(Simulation& simulation, TaskData* data, size_t count);

	void SimulationInitialize(Simulation& simulation);

	void SimulationUninitialize(Simulation& simulation);

	void SimulationUpdate(Simulation& simulation);
}