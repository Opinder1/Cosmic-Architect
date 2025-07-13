#pragma once

#include "Render/RenderAllocator.h"

#include "Spatial3D/SpatialPoly.h"
#include "Spatial3D/SpatialWorld.h"

#include "Entity/EntityPoly.h"

#include "Simulation/Config.h"

#include "Commands/TypedCommandBuffer.h"

#include "Util/Span.h"

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

	// Per thread data
	struct ThreadContext
	{
		// Rendering
		TCommandBuffer<RS> commands;

		rendering::Allocator allocator;
	};

	struct Simulation
	{
		// Simulation
		bool high_priority = true;
		bool thread_mode = false;
		bool unloading = false;

		uint8_t processor_count = 0;
		uint8_t worker_count = 1;

		uint64_t frame_index = 0;
		Clock::time_point frame_start_time;

		godot::String path;

		simulation::Config config;
		Clock::time_point last_config_save;

		std::vector<ThreadContext> thread_contexts;

		// Spatial
		std::vector<spatial3d::WorldPtr> spatial_worlds;
		std::vector<spatial3d::ScalePtr> spatial_scales;

		spatial3d::TypeData universe_type;
		spatial3d::TypeData galaxy_type;
		spatial3d::TypeData star_system_type;
		spatial3d::TypeData planet_type;
		spatial3d::TypeData space_station_type;
		spatial3d::TypeData space_ship_type;
		spatial3d::TypeData vehicle_type;

		// Entity
		entity::Factory entity_factory; // These entities are being updated every frame

		std::vector<entity::Ref> updating_entities;

		// Rendering
		std::vector<rendering::ThreadContext> rendering_contexts;

		// Universe
		std::vector<entity::Ref> universes;

		// Galaxy
		std::vector<entity::Ref> galaxies;

		// StarSystem
		std::vector<entity::Ref> star_systems;

		// Network
		godot::Ref<godot::X509Certificate> server_certificate;
		godot::Ref<godot::UDPServer> server_udp;
		godot::Ref<godot::DTLSServer> server_dtls;
	};

	simulation::ConfigDefaults GetConfigDefaults();

	void SimulationDoTasks(Simulation& simulation, TaskData& task_data);
	void SimulationDoMultitasks(Simulation& simulation, Span<TaskData> task_data);

	void SimulationInitialize(Simulation& simulation);
	void SimulationUnload(Simulation& simulation);
	void SimulationUninitialize(Simulation& simulation);
	void SimulationUpdate(Simulation& simulation);
}