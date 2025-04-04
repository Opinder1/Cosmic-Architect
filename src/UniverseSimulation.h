#pragma once

#include "Spatial3D/SpatialPoly.h"

#include "Entity/EntityPoly.h"

namespace voxel_game
{
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

		// Spatial
		std::vector<spatial3d::ScaleRef> spatial_scales;
		std::vector<spatial3d::WorldRef> spatial_worlds;

		spatial3d::Types universe_types;
		spatial3d::Types galaxy_types;
		spatial3d::Types star_system_types;
		spatial3d::Types planet_types;
		spatial3d::Types space_station_types;
		spatial3d::Types space_ship_types;
		spatial3d::Types vehicle_types;

		std::vector<spatial3d::ScaleRef> universe_scales;
		std::vector<spatial3d::ScaleRef> galaxy_scales;
		std::vector<spatial3d::ScaleRef> star_system_scales;
		std::vector<spatial3d::ScaleRef> planet_scales;
		std::vector<spatial3d::ScaleRef> space_station_scales;
		std::vector<spatial3d::ScaleRef> space_ship_scales;
		std::vector<spatial3d::ScaleRef> vehicle_scales;

		std::vector<spatial3d::WorldRef> universe_worlds;
		std::vector<spatial3d::WorldRef> galaxy_worlds;
		std::vector<spatial3d::WorldRef> star_system_worlds;
		std::vector<spatial3d::WorldRef> planet_worlds;
		std::vector<spatial3d::WorldRef> space_station_worlds;
		std::vector<spatial3d::WorldRef> space_ship_worlds;
		std::vector<spatial3d::WorldRef> vehicle_worlds;

		// Universe
		entity::Ref universe;

		// Galaxy
		std::vector<entity::Ref> galaxies;

		// StarSystem
		std::vector<entity::Ref> star_systems;
	};

	void DoTasks(Simulation& simulation, void(*callback)(Simulation&, size_t), size_t count);

	void Initialize(Simulation& simulation);

	void Uninitialize(Simulation& simulation);

	void Update(Simulation& simulation);
}