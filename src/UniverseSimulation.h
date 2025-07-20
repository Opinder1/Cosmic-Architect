#pragma once

#include "Modules.h"

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
		// Entity
		std::vector<entity::Ref> load_commands;
		std::vector<entity::Ref> unload_commands;

		// Rendering
		TCommandBuffer<RS> commands;

		rendering::Allocator allocator;
	};

	// Extra for spatial types
	struct SpatialTypeData : spatial3d::TypeData
	{
		std::vector<void(*)(Simulation&, spatial3d::WorldPtr)> world_updates;
		std::vector<void(*)(Simulation&, spatial3d::ScalePtr)> scale_updates;

		// Arrays of worlds and scales of this type for tasks to reference
		std::vector<spatial3d::WorldPtr> worlds;
		std::vector<spatial3d::ScalePtr> scales;
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

		std::vector<Module> modules;

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

		// Entity
		entity::Factory entity_factory; // These entities are being updated every frame

		std::vector<entity::Ref> updating_entities;

		// Entity lists
		std::vector<entity::WRef> universes;
		std::vector<entity::WRef> galaxies;
		std::vector<entity::WRef> star_systems;
		std::vector<entity::WRef> planets;
		std::vector<entity::WRef> space_stations;
		std::vector<entity::WRef> space_ships;
		std::vector<entity::WRef> vehicles;

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

	entity::Ref SimulationCreateEntity(Simulation& simulation, UUID id, entity::TypeID types);
	void SimulationUnloadEntity(Simulation& simulation, entity::WRef entity);
}