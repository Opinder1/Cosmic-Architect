#pragma once

#include "Spatial3D/SpatialPoly.h"

#include "Entity/EntityPoly.h"

namespace voxel_game::universe
{
	struct Simulation
	{
		// Spatial
		spatial3d::Types universe_types;
		spatial3d::Types galaxy_types;
		spatial3d::Types star_system_types;
		spatial3d::Types planet_types;
		spatial3d::Types space_station_types;
		spatial3d::Types space_ship_types;
		spatial3d::Types vehicle_types;

		std::vector<spatial3d::ScaleRef> spatial_scales;
		std::vector<spatial3d::ScaleRef> universe_scales;
		std::vector<spatial3d::ScaleRef> galaxy_scales;
		std::vector<spatial3d::ScaleRef> star_system_scales;
		std::vector<spatial3d::ScaleRef> planet_scales;
		std::vector<spatial3d::ScaleRef> space_station_scales;
		std::vector<spatial3d::ScaleRef> space_ship_scales;
		std::vector<spatial3d::ScaleRef> vehicle_scales;

		std::vector<spatial3d::WorldRef> spatial_worlds;
		std::vector<spatial3d::WorldRef> universe_worlds;
		std::vector<spatial3d::WorldRef> galaxy_worlds;
		std::vector<spatial3d::WorldRef> star_system_worlds;
		std::vector<spatial3d::WorldRef> planet_worlds;
		std::vector<spatial3d::WorldRef> space_station_worlds;
		std::vector<spatial3d::WorldRef> space_ship_worlds;
		std::vector<spatial3d::WorldRef> vehicle_worlds;

		// Entity
		entity::Factory entity_factory;

		entity::Ptr universe;

		// Galaxy
		std::vector<entity::Ptr> galaxies;
	};
}