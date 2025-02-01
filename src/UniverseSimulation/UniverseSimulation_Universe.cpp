#include "UniverseSimulation.h"
#include "UniverseSimulation_StringNames.h"

#include "Simulation/SimulationComponents.h"

#include "Loading/LoadingComponents.h"

#include "Spatial3D/SpatialComponents.h"
#include "Spatial3D/SpatialModule.h"

#include "Galaxy/GalaxyComponents.h"

#include "Render/RenderComponents.h"

namespace voxel_game
{
	flecs::entity CreateNewSimulatedGalaxy(flecs::world& world, const godot::String& path, flecs::entity_t universe_entity)
	{
		// Create the simulated galaxy
		flecs::entity galaxy_entity = world.entity();

#if defined(DEBUG_ENABLED)
		galaxy_entity.set_name("SimulatedGalaxy");
#endif

		galaxy_entity.emplace<sim::Path>(path);

		loading::Database& database = galaxy_entity.ensure<loading::Database>();
		database.path = path.path_join("entities.db");

		galaxy_entity.child_of(universe_entity);

		galaxy_entity.add<galaxy::World>();

		spatial3d::WorldMarker& spatial_world = galaxy_entity.ensure<spatial3d::WorldMarker>();
		spatial_world.world.max_scale = spatial3d::k_max_world_scale;

		galaxy_entity.add<physics3d::Position>();
		galaxy_entity.add<physics3d::Rotation>();

		spatial3d::InitializeWorldScales(galaxy_entity, spatial_world);

		// We want the simulated galaxy to load all galaxies around it
		spatial3d::Loader& spatial_loader = galaxy_entity.ensure<spatial3d::Loader>();
		spatial_loader.dist_per_lod = 3;
		spatial_loader.min_lod = 0;
		spatial_loader.max_lod = spatial3d::k_max_world_scale;

		galaxy_entity.add<rendering::Transform>();

		return galaxy_entity;
	}

	godot::Dictionary UniverseSimulation::GetUniverseInfo()
	{
		std::shared_lock lock(m_info_cache.mutex);
		return m_info_cache.universe_info;
	}

	void UniverseSimulation::ConnectToGalaxyList(const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::ConnectToGalaxyList>(ip))
		{
			return;
		}
	}

	void UniverseSimulation::DisconnectFromGalaxyList()
	{
		if (DeferCommand<&UniverseSimulation::DisconnectFromGalaxyList>())
		{
			return;
		}
	}

	void UniverseSimulation::QueryGalaxyList(const godot::Dictionary& query)
	{
		if (DeferCommand<&UniverseSimulation::QueryGalaxyList>(query))
		{
			return;
		}
	}

	void UniverseSimulation::PingRemoteGalaxy(const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::PingRemoteGalaxy>(ip))
		{
			return;
		}
	}

	void UniverseSimulation::StartLocalGalaxy(const godot::String& path, const godot::String& fragment_type, ServerType server_type)
	{
		if (DeferCommand<&UniverseSimulation::StartLocalGalaxy>(path, fragment_type, server_type))
		{
			return;
		}

		m_galaxy_entity = CreateNewSimulatedGalaxy(m_world, path, m_universe_entity);

		QueueSignal(k_signals->connected_to_galaxy);
	}

	void UniverseSimulation::ConnectToGalaxy(const godot::String& path, const godot::String& ip)
	{
		if (DeferCommand<&UniverseSimulation::ConnectToGalaxy>(path, ip))
		{
			return;
		}
	}

	void UniverseSimulation::DisconnectFromGalaxy()
	{
		if (DeferCommand<&UniverseSimulation::DisconnectFromGalaxy>())
		{
			return;
		}

		ecs_delete(m_world, m_galaxy_entity);

		QueueSignal(k_signals->disconnected_from_galaxy);
	}
}